/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 * @file       server.c
 * @brief      con and destruct server and manage it.
 * 
 * @note       
 * @history:
 *   Version   Date            Author          Modification    Email
 *   V1.0.0    Oct-06-2023     Ethan Oliveira                  ethanjamesoliveira@gmail.com
 * 
 * @verbatim
 * ==============================================================================
 * 
 * ==============================================================================
 * @endverbatim
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 */
#include <stdbool.h>

bool DEBUG = false; // Debug mode

#include "Headers/server.h"
#include "Headers/browser.h"
#include "Headers/client.h"
#include "Headers/ccmds.h"
#include "Headers/tools.h"

PortDesc portList[] = {};

void ServerPrint(const char* color, const char* str, ...) {
    struct tm* timestr = gmt();

    va_list argp;
    va_start(argp, str);
    printf("%s", color);
    printf("[srv/][%02d:%02d:%02d] ", timestr->tm_hour, timestr->tm_min, timestr->tm_sec);

    // Prefix
    vfprintf(stdout, str, argp);
    va_end(argp);    
    printf(RESET "\n");
}

void* ListenForRequestsOnServer(void* server)
{
    Server* serverToListenOn = (Server*)server;

    while (1)
    {
        ServerRequest request = {0};

        // Recv info from the most recent client
        // New thread created every client. Bad but will do
        int recvBytes = recv(serverToListenOn->clientList[serverToListenOn->connectedClients-1]->cfd, (void*)&request, sizeof(request), 0);
        
        if (recvBytes <= 0)
            continue;
        
        request.requestMaker.cfd = serverToListenOn->clientList[serverToListenOn->connectedClients-1]->cfd;
        request.requestMaker.connectedServer = serverToListenOn;

        printf(CYN "[%s] Received Server Request: %i\n" RESET, serverToListenOn->alias, request.command);

        DoServerRequest(request);
        
        // Don't listen for requests from that client anymore
        if (request.command == k_cfKickClientFromServer)
            break;
    }
    pthread_exit(NULL);
}

/**
 * 
 * @brief           Accept all client connections on the server provided
 * @param[in]       serverInfo: the server to accept client connections for
 * @return          void*
 * @retval          None
 */
void* ServerAcceptThread(void* serverInfo)
{
    // Server info
    Server* server = (Server*)serverInfo; 

    pthread_t activeClientThreads[server->maxClients];
    int numOfClientThreads = -1;
    
    pthread_t activeClientServerThreads[server->maxClients];
    int numOfClientServerThreads = -1;

    int lstn = listen(server->sfd, server->maxClients);
    if (lstn < 0) {
        ServerPrint(RED, "Error Listening on Server %s", server->alias);
        close(server->sfd);
        return;
    }

    // 'server' becomes unsafe since we are in a thread.
    // make copy of 'server' by dereferencing it. dont even need it as a pointer anymore
    Server dereferencedServer = *server;

    int cfd = 0; // client file descriptor. get it from accepting the client
    while (1)
    {
        cfd = accept(dereferencedServer.sfd, NULL, NULL);

        if (cfd < 0) {
            continue;
        }

        User receivedUserInfo = {0};
        int clientInfo = recv(cfd, (void*)&receivedUserInfo, sizeof(User), 0);  

        if (clientInfo < 0)
            continue;
        else if (clientInfo == 0) // client disconnected
            break;

        /*
            Update the servers client list and since its dereferenced we 
            dont have to worry about putting inaccurate information
            in the actual server.
        */
        dereferencedServer.clientList[dereferencedServer.connectedClients] = &receivedUserInfo;
        dereferencedServer.connectedClients++;

        /*
            Update the clients connection info
        */
        receivedUserInfo.cfd             = cfd;
        receivedUserInfo.connectedServer = &dereferencedServer;

        // Send the server info
        int sentServerInfo = send(cfd, (void*)&dereferencedServer, sizeof(dereferencedServer), 0);
        if (sentServerInfo <= 0) // Error sending info
            break;
        
        pthread_t tid;
        if (pthread_create(&tid, NULL, ListenForRequestsOnServer, (void*)&dereferencedServer) != 0)
            break;
    }

    pthread_exit(NULL);
}

uint32_t GenerateServerUID(Server* server)
{
    // seed rand with the server name in int and port and address of serverAddr
    srand((unsigned int)server->alias ^ server->port ^ (unsigned int)(void*)&server->addr);

    uint32_t uid = (unsigned int)"UID" + (unsigned int)rand();
    server->serverId = uid;
    
    return uid;
}

/**
 * @brief           Construct a server using arguments found in the Thread_ServerArgs struct
 * @param[in]       serverArgsStruct: Thread_AcceptArgs struct with parameters
 * @return          void*
 */
void* RSServerBareMetal(void* serverStruct)
{

    /* 
     * Create struct representing server
     * Used to resolve information about a chatroom
     */
    ServerCreationInfo* creationInfo = (ServerCreationInfo*)serverStruct;
    Server*             serverInfo   = creationInfo->serverInfo; // The info the user provided about the server they want to make
    SysPrint(CYN, false, "%s: Request: Make New Server", creationInfo->clientAKAhost->handle);

    RootResponse response;
    memset(&response, 0, sizeof(RootResponse));
    response.rcode       = k_rcInternalServerError;
    response.returnValue = (void*)serverInfo;
    response.rflag       = k_rfSentDataWasUnused;

    if (serverInfo->maxClients > kMaxServerMembers){
        // Max clients is greater
        serverInfo->maxClients = kDefaultMaxClients;
    }

    /* Server address information */
    struct sockaddr_in addrInfo;
    memset(&addrInfo, 0, sizeof(struct sockaddr_in));
	addrInfo.sin_family      = serverInfo->domain;
	addrInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	addrInfo.sin_port        = htons(serverInfo->port);

    // Create server socket
    int sfd = socket(serverInfo->domain, serverInfo->type, 0);
    if (sfd == -1) {
        ServerPrint(RED, "[ERROR]: Failed Creating Socket. Error Code %i", errno);
        RSRespondToRootRequestMaker(creationInfo->clientAKAhost, response);
        goto server_close;
    }
    
    /*
        We set SO_REUSEADDR to true, in case a server
        that previously used this port is closed but 
        we cant create another socket with this port because of
        the TIME_WAIT state in the kernel.
    */
    int optVal = 1;
    int set = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
    if (set < 0) {
        ServerPrint(RED, "setsockopt() for SO_REUSEADDR failed. Maybe port is in use.");
        RSRespondToRootRequestMaker(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    // Bind socket to the address info provided
    int bnd = bind(sfd, (struct sockaddr * )&addrInfo, sizeof(addrInfo));
    if (bnd == -1) {
        ServerPrint(RED, "[ERROR]: Failed Binding Socket. Error Code %d", errno);
        RSRespondToRootRequestMaker(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    // Listen for client connections
    int lsn = listen(sfd, serverInfo->maxClients);
    if (lsn == -1) {
        ServerPrint(RED, "[ERROR]: Error while listening. Error Code %d", errno);
        RSRespondToRootRequestMaker(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    /* Server online, fill in rest of info */
    User hostCopy = *creationInfo->clientAKAhost;

    GenerateServerUID(serverInfo);
    serverInfo->connectedClients = 0;
    serverInfo->host             = hostCopy;
    serverInfo->domain           = creationInfo->serverInfo->domain;
    serverInfo->isRoot           = false;
    serverInfo->port             = serverInfo->port;
    serverInfo->type             = serverInfo->type;
    serverInfo->maxClients       = creationInfo->serverInfo->maxClients;
    serverInfo->sfd              = sfd;
    serverInfo->addr             = addrInfo;
    serverInfo->online           = true; // True. Server online and ready
    serverInfo->clientList       = (User**)malloc(sizeof(User*) * serverInfo->maxClients); // Allocate memory for the servers client list
    strcpy(serverInfo->alias, creationInfo->serverInfo->alias);
    
    // add server to server list
    serverList[onlineServers++] = *serverInfo;

    // respond to host telling them their server was made
    // update their client info server sided
    
    hostCopy.connectedServer = serverInfo;
    SSUpdateClientWithNewInfo(hostCopy);

    response.rcode       = k_rcRootOperationSuccessful;
    response.returnValue = (const void*)serverInfo;
    response.rflag       = k_rfRequestedDataUpdated;
    RSRespondToRootRequestMaker(&serverInfo->host, response);

    pthread_t tid;
    pthread_create(&tid, NULL, ServerAcceptThread, (void*)serverInfo);
    pthread_join(tid, NULL);
    return;

server_close:
    close(sfd);
    pthread_exit(NULL);
    return;
}

/** 
 * @brief           Wrapper for RSServerBareMetal()
 * @param[in]       domain:     Domain for the socket
 * @param[in]       type:       Type of the socket
 * @param[in]       protocol:   Protocol for the socket
 * @param[in]       port:       Port for the socket
 * @param[in]       ip:         Ip to host server on
 * @param[in]       maxClients: Max clients able to connect
 * @param[in]       alias:      Name of the server
 * @return          int
 * @retval          Server Status or Setup status
 */
int MakeServer(
    int domain, // AF_INET
    int type, // SOCK_STREAM
    int protocol, // USUALLY ZERO
    int port,
    unsigned int maxClients,
    char* alias
)
{
    // Make sure server name follows rules
    if (strlen(alias) < kMinServerAliasLength){
        SysPrint(YEL, true, "[WARNING]: Server Name Too Short. Min %i", kMinServerAliasLength);
        return -1;
    }

    if (strlen(alias) > kMaxServerAliasLength){
        SysPrint(YEL, true, "[WARNING]: Server Name Longer Than (%i) Chars.", kMaxServerAliasLength);
        return -1;
    }

    if (strstr(alias, " ") != NULL){
        SysPrint(YEL, true, "[WARNING]: Server Name Cannot Have Spaces. Select a Different Name.");
        return -1;
    }

    // Iterate through servers, Check if the alias, and port is already in use
    // TODO: Optimize. Might be extremely slow when theres a lot of servers
    // O(n). Slow
    
    // for (int i = 0; i < onlineServers; i++){
        
    //     // Check if port is in use
    //     if (port == serverList[i].port)
    //     {
    //         SysPrint(YEL, true, "[WARNING]: Port Already In Use. Choose a Different Port. (%i)", port);
    //         return -1;
    //     }
        
    //     // Check if alias is in use
    //     char currentServerName[kMaxServerAliasLength + 1];
    //     strcpy(currentServerName, serverList[i].alias);
    //     if (currentServerName != NULL){
    //         toLowerCase(alias); 
    //         toLowerCase(currentServerName);

    //         if (strcmp(currentServerName, alias) == 0) { // Server name is already used
    //             SysPrint(YEL, true, "[WARNING]: Cannot Create Server: Name Already In Use. (%s)", alias);
    //             return -1;
    //         }
    //     }
    // }
    
    Server serv;
    memset(&serv, 0, sizeof(Server));

    // Set the server properties to be created
    serv.domain     = domain;
    serv.type       = type;
    serv.protocol   = protocol;
    serv.port       = port;
    serv.maxClients = maxClients;
    serv.isRoot     = false;
    serv.host       = *localClient;
    strcpy(serv.alias, alias);
    
    printf("host: %s\n", serv.host.handle);

    // Tell root server to host a server
    RootResponse response = MakeRootRequest(k_cfMakeNewServer, serv, *localClient, (CMessage){0});
    if (response.rcode == k_rcRootOperationSuccessful) 
    {
        localClient->connectedServer = &serv;
        SysPrint(CYN, false, "Server '%s' Created on Port '%i'\n", serv.alias, serv.port);
    }
    else if (response.rcode == k_rcInternalServerError && response.returnValue == (void*)serv.port)
        // Port in use error
        SysPrint(YEL, false, "[WARNING]: Port Already In Use. Choose a Different Port. (%i)\n", serv.port);
    else
        SysPrint(RED, false, "Error Making Server '%s'\n", serv.alias);

    return response.rcode;
}

bool IsUserHost(User user, Server* server)
{
    return (strcmp(user.handle, server->host.handle) == 0);
}

void ShutdownServer(Server* server)
{
    printf("Server shutdown requested for '%s'...\n", server->alias);
    
    if (shutdown(server->sfd, SHUT_RDWR) < 0)
        printf("Error Calling 'shutdown()' for Server. Error Code %i\n", errno);

    server->online = false;

    // Unuse port
    int portIndex = server->port % kMaxServersOnline;
    portList[portIndex].inUse = false;

    printf("Disconnecting all clients from server... ");
    for (int cl_index=0; cl_index<server->connectedClients; cl_index++)
    {
        close(server->clientList[cl_index]->cfd);
        memset(server->clientList[cl_index]->connectedServer, 0, sizeof(Server));
    }

    printf("Done\n");
    printf("Removing server from server list... ");
    int serverIndex = -1;

    // Get index where the server is located on serverList
    for (int i=0; i < onlineServers; i++){
        if (strcmp(serverList[i].alias, server->alias) == 0){
            serverIndex = i;
            break;
        }        
    }

    // Remove the server from the server list
    if (serverIndex != -1){
        for (int i = serverIndex; i < onlineServers - 1; i++)
            serverList[i] = serverList[i + 1];

        onlineServers--;
    }

    printf("Done\n");
    printf("Closing server socket and freeing memory... ");

    close(server->sfd);
    printf("Done\n");
    printf("Server closed successfully... Done\n");
}

void EncryptClientMessage(CMessage* message)
{
    // TODO: Use aes lib

    const int kXorConstant = message->cflag;

    for (int charIndex=0; charIndex<strlen(message->message); charIndex++)
    {
        message->message[charIndex] = message->message[charIndex] ^ kXorConstant;
    }
}

ResponseCode DoServerRequest(ServerRequest request)
{
    User sender = request.requestMaker;
    
    ResponseCode responseStatus = k_rcInternalServerError;

    switch (request.command)
    {
    case k_cfKickClientFromServer:
        SSDisconnectClientFromServer(&sender);
        sender.connectedServer = &rootServer;
        break;
    case k_cfEchoClientMessageInServer:
        Server* connectedServer = sender.connectedServer;

        EncryptClientMessage(&request.optionalClientMessage);

        // relay encrypted message to all connected clients
        for (int ci = 0; ci < connectedServer->connectedClients; ci++)
        {
            if (sender.cfd == connectedServer->clientList[ci]->cfd)
                continue; // Dont send the message to the client who sent the message

            int sentBytes = send(connectedServer->clientList[ci]->cfd,
                                 (void*)&request.optionalClientMessage, 
                                 sizeof(request.optionalClientMessage), 0);    
        }

        responseStatus = k_rcRootOperationSuccessful;

        // Send the response status back to the user
        int sent = send(sender.cfd, (void*)&responseStatus, sizeof(responseStatus), 0);
        if (sent <= 0)
        {
            SSDisconnectClientFromServer(&sender);
            return;
        }

        break;
    default:
        break;
    }

    return responseStatus;
}

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

#include "hdr/server.h"
#include "hdr/browser.h"
#include "hdr/client.h"
#include "hdr/ccmds.h"
#include "hdr/tools.h"

void SERVER_PRINT(const char* color, const char* str, ...) {
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

void* ServerRecvThread(void* serverInfo)
{
    Server* server = (Server*)serverInfo;
    while (1)
    {
        // Receive a message from a client
        CMessage message = {0};
        
        int receivedBytes = recv(localClient->cfd, (void*)&message, sizeof(message), 0);
        if (receivedBytes <= 0)
            break; // TODO: handle situation
        
        if (message.cflag == k_cfEchoClientMessageInServer)
        {

            // Relay that message to all other clients so they can see what the client said
            for (int ci = 0; ci < server->connectedClients; ci++)
            {
                CMessage relayThisMessage = {0};
                relayThisMessage.cflag    = k_cfPrintPeerClientMessage;
                relayThisMessage.sender   = message.sender;


                // TODO: ENCRYPTED THE MESSAGE
                // KEY SHOULD BE THE TIME THE USER JOINED THE ROOT SERVER IN MS + 12


                strcpy(relayThisMessage.message, message.message);

                int sentBytes = send(server->clientList[ci]->cfd, (void*)&relayThisMessage, sizeof(CMessage), 0);
                if (sentBytes <= 0)
                    continue;
            }
        }

    }
}

void RecvMessages(void* serverInfo)
{
    Server* server = (Server*)serverInfo;

    while (1)
    {   
        for (int c = 0; c < server->connectedClients; c++)
        {
            CMessage message = {0};
            int receivedBytes = recv(server->clientList[c]->cfd, (void*)&message, sizeof(message), MSG_DONTWAIT);
            
            printf("Received message from %s.\n", message.sender->handle);

            // Send that message to all clients
            if (message.cflag == k_cfEchoClientMessageInServer && receivedBytes > 0)
            {
                // FIXME: Make it a thread so it can relay multiple messages at a time
                // TODO: ENCRYPT MESSAGES
                // Relay received message to all clients
                for (int ci = 0; ci < server->connectedClients; ci++) {
                    User* connectedClient = server->clientList[ci];

                    // Dont send the message back to the person who actually SENT the message in the first place
                    if (strcmp(connectedClient->handle, message.sender->handle) == 0)
                        continue;

                    // VITAL: Tell the client we want them to recv and print it out
                    message.cflag = k_cfPrintPeerClientMessage;

                    int sentBytes = sendto(connectedClient->cfd, (void*)&message, sizeof(message), 0, (struct sockaddr*)&connectedClient->caddr, sizeof(connectedClient->caddr));
                    // TODO: handle sendto error situations            
                }
            }
        }
        
    }

}

void RelayMessagesOnServer(void* serverInfo)
{
    // Receive cmessages and then send them to all clients
    Server* server = (Server*)serverInfo;
    pthread_t pid;
    pthread_create(&pid, NULL, RecvMessages, (void*)server);
    pthread_join(pid, NULL);
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

        DoServerRequest((void*)&request);
        
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
        printf("listen: errno %i\n", errno);
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
            // printf("accept: errno %i\n", errno);
            // close(server->sfd);
            continue;
        }

        User receivedUserInfo = {0};
        int clientInfo = recv(cfd, (void*)&receivedUserInfo, sizeof(User), 0);  

        if (clientInfo < 0)
            continue;
        else if (clientInfo == 0) // client disconnected
            break;

        dereferencedServer.clientList[dereferencedServer.connectedClients] = &receivedUserInfo;
        dereferencedServer.connectedClients++;

        receivedUserInfo.cfd = cfd;
        receivedUserInfo.connectedServer = &dereferencedServer;

        pthread_t tid;
        pthread_create(&tid, NULL, ListenForRequestsOnServer, (void*)&dereferencedServer);

        // Send the server info
        int sentServerInfo = send(cfd, (void*)&dereferencedServer, sizeof(dereferencedServer), 0);

        // TODO: handle situation where send fails
    }

    pthread_exit(NULL);
}

void MallocServerClientList(Server* server)
{
    server->clientList = (User**)malloc(sizeof(User*) * server->maxClients);
}

/**
 * @brief           Construct a server using arguments found in the Thread_ServerArgs struct
 * @param[in]       serverArgsStruct: Thread_AcceptArgs struct with parameters
 * @return          void*
 */
void* ServerBareMetal(void* serverStruct)
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
    response.returnValue = NULL;
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
        SERVER_PRINT(RED, "[ERROR]: Failed Creating Socket. Error Code %i", errno);
        RespondToRootRequestMaker(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    // Bind socket to the address info provided
    int bnd = bind(sfd, (struct sockaddr * )&addrInfo, sizeof(addrInfo));
    if (bnd == -1) {
        SERVER_PRINT(RED, "[ERROR]: Failed Binding Socket. Error Code %d", errno);
        RespondToRootRequestMaker(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    // Listen for client connections
    int lsn = listen(sfd, serverInfo->maxClients);
    if (lsn == -1) {
        SERVER_PRINT(RED, "[ERROR]: Error while listening. Error Code %d", errno);
        RespondToRootRequestMaker(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    /* Server online, fill in rest of info */
    User hostCopy = *creationInfo->clientAKAhost;

    serverInfo->connectedClients = 0;
    serverInfo->host       = &hostCopy;
    serverInfo->domain     = creationInfo->serverInfo->domain;
    serverInfo->isRoot     = false;
    serverInfo->port       = serverInfo->port;
    serverInfo->type       = serverInfo->type;
    serverInfo->maxClients = creationInfo->serverInfo->maxClients;
    serverInfo->sfd        = sfd;
    serverInfo->addr       = addrInfo;
    serverInfo->online     = true; // True. Server online and ready
    MallocServerClientList(serverInfo);
    strcpy(serverInfo->alias, creationInfo->serverInfo->alias);
    
    // add server to server list
    serverList[onlineServers++] = *serverInfo;

    // respond to host telling them their server was made
    response.rcode = k_rcRootOperationSuccessful;
    response.returnValue = NULL;
    response.rflag = k_rfRequestedDataUpdated;
    RespondToRootRequestMaker(serverInfo->host, response);

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
 * @brief           Wrapper for ServerBareMetal()
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
    for (int i = 0; i < onlineServers; i++){
        
        // Check if port is in use
        if (port == serverList[i].port)
        {
            SysPrint(YEL, true, "[WARNING]: Port Already In Use. Choose a Different Port. (%i)", port);
            return -1;
        }
        
        // Check if alias is in use
        char currentServerName[kMaxServerAliasLength + 1];
        strcpy(currentServerName, serverList[i].alias);
        if (currentServerName != NULL){
            toLowerCase(alias); 
            toLowerCase(currentServerName);

            if (strcmp(currentServerName, alias) == 0) { // Server name is already used
                SysPrint(YEL, true, "[WARNING]: Cannot Create Server: Name Already In Use. (%s)", alias);
                return -1;
            }
        }
    }
    
    Server serv;
    memset(&serv, 0, sizeof(Server));

    // Set the server properties to be created
    serv.domain     = domain;
    serv.type       = type;
    serv.protocol   = protocol;
    serv.port       = port;
    serv.maxClients = maxClients;
    serv.isRoot     = false;
    strcpy(serv.alias, alias);
    
    // Tell root server to host a server
    RootResponse response = MakeRootRequest(k_cfMakeNewServer, serv, *localClient, (CMessage){0});
    if (response.rcode == k_rcRootOperationSuccessful)
        SysPrint(CYN, false, "Server '%s' Created on Port '%i'\n", serv.alias, serv.port);
    else
        SysPrint(RED, false, "Error Making Server '%s'\n", serv.alias);

    return response.rcode;
}

bool IsUserHost(User user, Server* server)
{
    return (strcmp(user.handle, server->host->handle) == 0);
}

void ShutdownServer(char* alias)
{
    printf("Server shutdown requested for '%s'...\n", alias);
    
    Server* server = ServerFromAlias(alias);
    server->online = false;
    
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
        if (strcmp(serverList[i].alias, alias) == 0){
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

// Send a message to all clients connected to the server
int SendServerMessage(Server* server, char* message)
{
    for (int cl_index = 0; cl_index < server->connectedClients; cl_index++)
    {
        if (write(server->clientList[cl_index]->cfd, message,
                 sizeof(message)) == -1)
        {
            SysPrint(RED, true, "Error sending server message. Errno %i", errno);
        }

    }
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

ResponseCode DoServerRequest(void* request)
{
    ServerRequest* serverRequest = (ServerRequest*)request;
    User sender = serverRequest->requestMaker;
    
    ResponseCode responseStatus = k_rcInternalServerError;

    switch (serverRequest->command)
    {
    case k_cfKickClientFromServer:
        DisconnectClientFromServer(&sender);
        break;
    case k_cfEchoClientMessageInServer:
        Server* connectedServer = sender.connectedServer;

        EncryptClientMessage(&serverRequest->optionalClientMessage);

        // relay encrypted message to all connected clients
        for (int ci = 0; ci < connectedServer->connectedClients; ci++)
        {
            if (sender.cfd == connectedServer->clientList[ci]->cfd)
                continue; // Dont send the message to the client who sent the message

            int sentBytes = send(connectedServer->clientList[ci]->cfd,
                                 (void*)&serverRequest->optionalClientMessage, 
                                 sizeof(serverRequest->optionalClientMessage), 0);    
        }

        responseStatus = k_rcRootOperationSuccessful;

        // Send the response status back to the user
        int sent = send(sender.cfd, (void*)&responseStatus, sizeof(responseStatus), 0);
        // todo: handle error situtations

        break;
    default:
        break;
    }

    return responseStatus;
}

int RelayClientSentMessage(
    Server* server,
    char* message, 
    User* op // Original Poster/Sender
)
{
    // No clients to relay msg to
    if (server->connectedClients <= 0)
        return -1;

    CMessage cmsg = {0};
    cmsg.cflag = k_cfPrintPeerClientMessage;
    cmsg.sender = op;
    strcpy(cmsg.message, message);
    ResponseCode requestStatus = MakeServerRequest(k_cfEchoClientMessageInServer, *op, cmsg);
  
    if (requestStatus == k_rcRootOperationSuccessful)
        return 0;

    return -1;
}


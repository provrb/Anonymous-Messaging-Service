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

/* Local client info */
User*  client;                      // Current client who ran the app

/* Root server info */
int    onlineGlobalClients = 0;         // All clients on the app
User   rootConnectedClients[] = {}; // All clients on the app must be connected to the root
Server rootServer;                  // Root server info

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
        
        int receivedBytes = recv(client->cfd, (void*)&message, sizeof(message), 0);
        if (receivedBytes <= 0)
            break; // TODO: handle situation
        
        if (message.cflag == k_cfEchoClientMessageInServer)
        {

            // Relay that message to all other clients so they can see what the client said
            for (int ci = 0; ci < server->connectedClients; ci++)
            {
                CMessage relayThisMessage = {0};
                relayThisMessage.cflag    = k_cfPrintEchodClientMessage;
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
                // Relay received message to all clients
                for (int ci = 0; ci < server->connectedClients; ci++) {
                    User* connectedClient = server->clientList[ci];

                    // Dont send the message back to the person who actually SENT the message in the first place
                    if (strcmp(connectedClient->handle, message.sender->handle) == 0)
                        continue;

                    // VITAL: Tell the client we want them to recv and print it out
                    message.cflag = k_cfPrintEchodClientMessage;

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
    printf("Real name: %s\n", server->alias);

    pthread_t activeClientThreads[server->maxClients];
    int numOfClientThreads = -1;
    
    pthread_t activeClientServerThreads[server->maxClients];
    int numOfClientServerThreads = -1;

    int cfd = 0; // client file descriptor. get it from accepting the client
    while (1)
    {
        cfd = accept(server->sfd, (struct sockaddr*)&server->addr, sizeof(server->addr));

        if (cfd < 0) continue;

        printf("\nAccepting a client connection on '%s'\n", server->alias);

        server->connectedClients++;
        
        // Create a new thread to handle the client connection
        pthread_t pid;

        if (server->isRoot)
        {
            activeClientThreads[numOfClientThreads++] = pid;
            if (pthread_create(&activeClientThreads[numOfClientThreads], NULL, ClientJoinedRoot, NULL) != 0) {
                SysPrint(RED, true, "Error Making Client Handler Thread.");
                DisconnectClient();
            }
        }
        else
        {
            printf("hello world\n");
            activeClientServerThreads[numOfClientServerThreads++] = pid;
            if (pthread_create(&activeClientServerThreads[numOfClientServerThreads], NULL, RelayMessagesOnServer, (void*)server) != 0) {
                SysPrint(RED, true, "Error Making Client Handler Thread.");
                DisconnectClient();
            }
        }
    }

    pthread_exit(NULL);
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
    Server* serverInfo = creationInfo->serverInfo;
    printf("User %s: Request: Make New Server\n", creationInfo->clientAKAhost->handle);
    // printf("Done\n");

    RootResponse response;
    memset(&response, 0, sizeof(RootResponse));
    response.rcode       = k_rcInternalServerError;
    response.returnValue = NULL;
    response.rflag       = k_rfSentDataWasUnused;

    if (serverInfo->maxClients > kMaxServerMembers){
        // Max clients is greater
        serverInfo->maxClients = kDefaultMaxClients;
    }

    // printf("Filling out server address information... ");

    /* Server address information */
    struct sockaddr_in addrInfo;
    memset(&addrInfo, 0, sizeof(struct sockaddr_in));
	addrInfo.sin_family      = serverInfo->domain;
	addrInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	addrInfo.sin_port        = htons(serverInfo->port);

    // printf("Done\n");
    // fprintf(stderr, "Creating socket for server...\n");
    // Create server socket
    int sfd = socket(serverInfo->domain, serverInfo->type, 0);
    if (sfd == -1) {
        SERVER_PRINT(RED, "[ERROR]: Failed Creating Socket. Error Code %i", errno);
        Respond(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    // printf("Done\n");
    // fprintf(stderr, "Binding socket to address info provided... ");

    // Bind socket to the address info provided
    int bnd = bind(sfd, (struct sockaddr * )&addrInfo, sizeof(addrInfo));
    if (bnd == -1) {
        SERVER_PRINT(RED, "[ERROR]: Failed Binding Socket. Error Code %d", errno);
        Respond(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    struct tm* gmttime = gmt();

    // " [00:00:00] Server 'my-room' Created By 'anon' "
    printf(CYN "\n[%02d:%02d:%02d] Server '%s' Created by '%s'\n" RESET, gmttime->tm_hour,
                                                                         gmttime->tm_min, 
                                                                         gmttime->tm_sec, 
                                                                         creationInfo->serverInfo->alias, 
                                                                         creationInfo->clientAKAhost->handle);

    // Listen for client connections
    int lsn = listen(sfd, serverInfo->maxClients);
    if (lsn == -1) {
        SERVER_PRINT(RED, "[ERROR]: Error while listening. Error Code %d", errno);
        Respond(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    /* Server online, fill in rest of info */
    serverInfo->connectedClients = 1; // 1 because the host
    serverInfo->host       = creationInfo->clientAKAhost;
    serverInfo->domain     = creationInfo->serverInfo->domain;
    serverInfo->isRoot     = false;
    serverInfo->port       = serverInfo->port;
    serverInfo->type       = serverInfo->type;
    serverInfo->maxClients = creationInfo->serverInfo->maxClients;
    serverInfo->sfd        = sfd;
    serverInfo->addr       = addrInfo;
    serverInfo->online     = true; // True. Server online and ready
    serverInfo->clientList = (User**)malloc(serverInfo->maxClients * sizeof(User*)); // initialize client list
    strcpy(serverInfo->alias, creationInfo->serverInfo->alias);

    // fprintf(stderr, "Appending server to browser... ");

    serverList[onlineServers++] = *serverInfo;
        
    // printf("Done\n");

    // fprintf(stderr, "Creating accept thread... \n");

    response.rcode = k_rcRootOperationSuccessful;
    response.returnValue = NULL;
    response.rflag = k_rfRequestedDataUpdated;
    Respond(creationInfo->clientAKAhost, response);

    pthread_t tid;
    pthread_create(&tid, NULL, ServerAcceptThread, (void*)serverInfo);
    pthread_join(tid, NULL);

server_close:
    close(sfd);
    pthread_exit(NULL);
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

    // Iterate through servers, check if the alias is already in use
    Server tmp;
    for (int i=0; i<onlineServers; i++){
        tmp = serverList[i];
        char tmpName[kMaxServerAliasLength + 1];
        strcpy(tmpName, tmp.alias);
        if (tmpName != NULL){
            toLowerCase(alias); 
            toLowerCase(tmpName);

            if (strcmp(tmpName, alias) == 0){ // The same
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
    RootResponse response = MakeRootRequest(k_cfMakeNewServer, serv, *client, (CMessage){0});
    if (response.rcode == k_rcRootOperationSuccessful)
        printf(GRN "Server '%s' Created on Port %i\n", serv.alias, serv.port);
    else
        printf(RED "Error Creating Server. Error Code %i\n", errno);

    return response.rcode;
}

bool IsUserHost(User* user, Server* server)
{
    return user == server->host;
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
    free(server);
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

int RelayClientSentMessage(
    Server* server,
    char* message, 
    User* op // Original Poster/Sender
)
{
    // No clients to relay msg to
    if (server->connectedClients <= 0)
        return -1;

    // Client isnt connected to the server they wish to relay the message in
    if (strcmp(op->connectedServer->alias, server->alias) != 0)
        return -1;
    
    // Prepare to send message using custom struct
    CMessage relayedMessage = {0};
    relayedMessage.cflag = k_cfPrintEchodClientMessage;
    relayedMessage.sender = op;
    strcpy(relayedMessage.message, message);

    printf("Before\n");

    for (int c_index = 0; c_index < server->connectedClients; c_index++)
    {
        User* connectedUser = server->clientList[c_index];
        // TODO: Uncomment this. Commented for debugging
        if (strcmp(connectedUser->handle, op->handle) == 0)
            continue;
        int sentBytes = send(connectedUser->cfd, (void*)&relayedMessage, sizeof(relayedMessage), 0);
    }
    printf("Good\n");
    // int sentBytes = send(op->cfd, (void*)&relayedMessage, sizeof(relayedMessage), MSG_NOSIGNAL);
    // if (errno == EPIPE) {
    //     printf("recovered from epipe. cfd %i\n", op->cfd);
    //     return -1;
    // }
    // if (sentBytes <= 0) {
    //     printf("error sending. errno %i\n", errno);
    //     return -1; // Error sending message
    // }

    return 0;
}
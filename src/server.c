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
bool   goodUsername = false; // Does the client username follow all rules

/* Root server info */
int    onlineWWClients = 0;         // All clients on the app
User   rootConnectedClients[] = {}; // All clients on the app must be connected to the root
Server rootServer;                  // Root server info

static void SERVER_PRINT(const char* color, const char* str, ...) {
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
        
        if (message.cflag == cf_RELAY_MESSAGE_IN_SERVER)
        {

            // Relay that message to all other clients so they can see what the client said
            for (int ci = 0; ci < server->connectedClients; ci++)
            {
                CMessage relayThisMessage = {0};
                relayThisMessage.cflag    = cf_RECV_CLIENT_SENT_MESSAGE;
                relayThisMessage.dflag    = df_CMESSAGE;
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
    pthread_t pid;

    pthread_t activeClientThreads[server->maxClients];
    int numOfClientThreads = -1;

    int cfd = 0; // client file descriptor. get it from accepting the client
    while (1)
    {
        cfd = accept(server->sfd, (struct sockaddr*)&server->addr, sizeof(server->addr));
        
        if (cfd < 0) continue;
        
        printf("\nAccepting a client connection on '%s'\n", server->alias);

        server->connectedClients++;
        
        // Create a new thread to handle the client connection
        pthread_t pid;
        activeClientThreads[numOfClientThreads++] = pid;

        if (server->isRoot)
        {
            if (pthread_create(&activeClientThreads[numOfClientThreads], NULL, ClientJoinedRoot, NULL) != 0) {
                SysPrint(RED, true, "Error Making Client Handler Thread.");
                DisconnectClient();
            }
                
        }
        else
        {
            if (pthread_create(&activeClientThreads[numOfClientThreads], NULL, ))
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
    printf("Creating server. Parsing info for server struct... ");
    ServerCreationInfo* creationInfo = (ServerCreationInfo*)serverStruct;
    Server* serverInfo = creationInfo->serverInfo;
    printf("Done\n");

    RootResponse response;
    memset(&response, 0, sizeof(RootResponse));
    response.rcode = rc_INTERNAL_SERVER_ERROR;
    response.returnValue = NULL;
    response.rflag = rf_NO_DATA_CHANGED;

    if (serverInfo->maxClients > NO_MORE_THAN){
        SERVER_PRINT(YEL, "[WARNING]: Max Clients Arg (%i) Higher Than Allowed (%i). Setting to 20.", serverInfo->maxClients, NO_MORE_THAN);
        serverInfo->maxClients = 20;
    }

    printf("Filling out server address information... ");

    /* Server address information */
    struct sockaddr_in addrInfo;
    memset(&addrInfo, 0, sizeof(struct sockaddr_in));
	addrInfo.sin_family      = serverInfo->domain;
	addrInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	addrInfo.sin_port        = htons(serverInfo->port);

    printf("Done\n");
    fprintf(stderr, "Creating socket for server...\n");
    // Create server socket
    int sfd = socket(serverInfo->domain, serverInfo->type, 0);
    if (sfd == -1) {
        SERVER_PRINT(RED, "[ERROR]: Failed Creating Socket. Error Code %i", errno);
        Respond(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    printf("Done\n");
    fprintf(stderr, "Binding socket to address info provided... ");

    // Bind socket to the address info provided
    int bnd = bind(sfd, (struct sockaddr * )&addrInfo, sizeof(addrInfo));
    if (bnd == -1) {
        SERVER_PRINT(RED, "[ERROR]: Failed Binding Socket. Error Code %d", errno);
        Respond(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    printf("DoThreadne\n");
    struct tm* gmttime = gmt();

    printf("Server Successfully Created @ %02d:%02d:%02d GMT:\n", gmttime->tm_hour, gmttime->tm_min, gmttime->tm_sec);

    // Listen for client connections
    int lsn = listen(sfd, serverInfo->maxClients);
    if (lsn == -1) {
        SERVER_PRINT(RED, "[ERROR]: Error while listening. Error Code %d", errno);
        Respond(creationInfo->clientAKAhost, response);
        goto server_close;
    }

    /* Server online, fill in rest of info */
    serverInfo->host       = creationInfo->clientAKAhost;
    serverInfo->connectedClients = 1; // 1 because the host
    serverInfo->dflag = df_SERVER;
    serverInfo->domain = creationInfo->serverInfo->domain;
    serverInfo->isRoot = false;
    serverInfo->port = creationInfo->serverInfo->port;
    serverInfo->type = creationInfo->serverInfo->type;
    strcpy(serverInfo->alias, creationInfo->serverInfo->alias);
    serverInfo->maxClients = creationInfo->serverInfo->maxClients;
    serverInfo->sfd        = sfd;
    serverInfo->addr       = addrInfo;
    serverInfo->online     = true; // True. Server online and ready
    serverInfo->clientList = (User**)malloc(serverInfo->maxClients * sizeof(User*)); // initialize client list

    fprintf(stderr, "Appending server to browser... ");

    serverList[onlineServers++] = *serverInfo;
        
    printf("Done\n");
    printf("Responding to client with updates... ");
    response.rcode = rc_SUCCESSFUL_OPERATION;
    response.returnValue = (void*)serverInfo;
    response.rflag = rf_DATA_UPDATED;
    Respond(serverInfo->host, response);

    printf("Responded\n");
    fprintf(stderr, "Creating accept thread... \n");

    ServerAcceptThread((void*)serverInfo);

    pthread_exit(NULL);
    printf("Done\n");
    printf("Accept thread done.\n");

server_close:
//     close(sfd);
//     pthread_exit(NULL);
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
    if (strlen(alias) < MIN_ALIAS_LEN){
        SysPrint(YEL, true, "[WARNING]: Server Name Too Short. Min %i", MIN_ALIAS_LEN);
        return -1;
    }

    if (strlen(alias) > MAX_ALIAS_LEN){
        SysPrint(YEL, true, "[WARNING]: Server Name Longer Than (%i) Chars.", MAX_ALIAS_LEN);
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
        char tmpName[MAX_ALIAS_LEN + 1];
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
    serv.dflag      = df_SERVER;
    serv.domain     = domain;
    serv.type       = type;
    serv.protocol   = protocol;
    serv.port       = port;
    serv.maxClients = maxClients;
    serv.isRoot     = false;
    strcpy(serv.alias, alias);

    
    // Tell root server to host a server
    RootResponse response = MakeRootRequest(cf_MAKE_NEW_SERVER, df_NONE, serv, *client, NULL);
    if (response.rcode == rc_SUCCESSFUL_OPERATION)
        printf("Server created on port %i...\n", serv.port);
    else
        printf("Error creating server...\n");

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
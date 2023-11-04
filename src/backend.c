/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 * @file       backend.cpp
 * @brief      all backend operations
 * 
 * @note       
 * @history:
 *   Version   Date            Author          Modification    Email
 *   V1.0.0    Oct-07-2023     Ethan Oliveira                  ethanjamesoliveira@gmail.com
 * 
 * @verbatim
 * ==============================================================================
 * 
 * ==============================================================================
 * @endverbatim
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 */

#include "hdr/backend.h"
#include "hdr/browser.h"
#include <netdb.h>

/**
 * @brief           Get a Server struct from a server alias(name)
 * @param[in]       alias: Name of the server to get info on
 * @return          Server
 * @retval          Struct of info about the server
 */
Server* ServerFromAlias(char* alias) {
    for (int i=0; i < onlineServers; i++){
        Server* server = &serverList[i];

        if (server->alias != NULL){
            toLowerCase(alias); 
            toLowerCase(server->alias);

            // Compare everything lowercased
            if (strcmp(server->alias, alias) == 0)
                return server;
        }
    }
    return NULL;
}

/**
 * @brief           Make a request from the client to the root server. Kind of like http
 * @param[in]       commandFlag:   tell the server what to do with the data
 * @param[in]       dataFlag:      ensure the server knows which data is being passed
 * @param[in]       currentServer: server to make the request from
 * @param[in]       relatedClient: client who made the request
 * @return          void*
 * @retval          RootResponse from server as void*
 */
RootResponse MakeRootRequest(
    cflags    commandFlag,
    dflags    dataFlag,
    Server    currentServer,
    User      relatedClient,
    CMessage* clientMessageInfo
)
{
    RootRequest request;
    request.cmdFlag           = commandFlag;
    request.dflag             = dataFlag;
    request.server            = currentServer;
    request.user              = relatedClient;
    request.clientSentMessage = (clientMessageInfo == NULL ? (CMessage){0} : *clientMessageInfo);

    // Default response values
    RootResponse response = {0};
    response.rcode        = rc_INTERNAL_SERVER_ERROR;
    response.rflag        = rf_NONE;
    response.returnValue  = NULL;

    // Send request to root server
    int sentBytes = send(rootServer.sfd, (const void*)&request, sizeof(RootRequest), 0);
    if (sentBytes <= 0) { // Client disconnected or something went wrong sending
        printf(RED "Error making request to root server...\n" RESET);
        return response;
    }

    // Client wont be able to receive messages when their socket file descriptor is closed
    if (request.cmdFlag == cf_DISCONNECT_CLIENT_FROM_ROOT)
        return response;

    // Receive a response from the root server
    int receivedBytes = recv(rootServer.sfd, (void*)&response, sizeof(RootResponse), 0);
    if (receivedBytes < 0) { // Client disconnected or something went wrong receiving
        printf(RED "Failed to receive data from root server...\n" RESET);
        return response;
    }
 
    // In the case of special commands
    // where we may need to send or recv more than once
    switch (request.cmdFlag)
    {
    case cf_REQUEST_SERVER_LIST: // Need to receive all servers individually
    {
        // Receive the number of online servers
        uint32_t onlineServersTemp = 0;
        int initialReceivedBytes = recv(rootServer.sfd, &onlineServersTemp, sizeof(onlineServersTemp), 0);
        
        if (initialReceivedBytes <= 0)
            break;
        
        onlineServers = ntohl(onlineServersTemp);

        // Receive all servers
        for (int i = 0; i < onlineServers; i++){
            Server receivedServer = {0};
            int receive = recv(rootServer.sfd, (void*)&receivedServer, sizeof(Server), 0);
            
            if (receive <= 0)
                break;
            
            // Update server list
            serverList[i] = receivedServer;
        }

        break;
    }
    default:
        break;
    }

    return response;
}

/**
 *    --------[ SETTINGS FUNCTIONS ]--------
 * Functions to control the settings
 * for the user. 
 * 
 * Settings reset every session.z
 * 
 */
CommandFunction EnableDebugMode(){    
    DEBUG = !DEBUG;
    (DEBUG ? SysPrint(YEL, true, "Debug Mode On.", DEBUG) : SysPrint(YEL, true, "Debug Mode Off.") ); 
}

CommandFunction ExitApp(){ 
    DisconnectClient();
    SysPrint(CYN, true, "Exiting Application.");
    exit(EXIT_SUCCESS); 
}

/**
 * 
 *      --------[ JOIN FUNCTIONS ]--------         
 * Many different ways a user can join a server
 * Used with commands and arguements.
 * 
 */
CommandFunction* JoinServerByName(void* name){ // Join server from its alias
    Server* server = ServerFromAlias((char*)name);
    if (!server){
        SysPrint(RED, true, "No Server Found With That Name.");
        return NULL;
    }

    printf("dbg: Server sfd %i server name %s\n", server->sfd, server->alias);
    int cfd = socket(server->domain, server->type, server->protocol);
    if (cfd < 0){
        SysPrint(RED, true, "Failed to create client socket. Errno %i", errno);
        return NULL;
    }

    int cnct = connect(cfd, (struct sockaddr*)&server->addr, sizeof(server->addr));
    if (cnct < 0){
        SysPrint(RED, true, "Failed to Connect to Server. Error %i", errno);
    }

    client->cfd = cfd;
    client->connectedServer = server;

    return NULL;
}

CommandFunction MakeServerCL() {

}

CommandFunction* JoinServerByListIndex(int index) {

}


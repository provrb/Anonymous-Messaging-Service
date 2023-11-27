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
 *    --------[ SETTINGS FUNCTIONS ]--------
 * Functions to control the settings
 * for the user. 
 * 
 * Settings reset every session.z
 * 
 */
// void EnableDebugMode(){    
//     DEBUG = !DEBUG;
//     (DEBUG ? SysPrint(YEL, true, "Debug Mode On.", DEBUG) : SysPrint(YEL, true, "Debug Mode Off.") ); 
// }

void ExitApp(){ 
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
void* JoinServer(Server* server) {
    int cfd = socket(server->domain, server->type, server->protocol);
    if (cfd < 0){
        SysPrint(RED, true, "Failed to create client socket. Errno %i", errno);
        return;
    }

    int cnct = connect(cfd, (struct sockaddr*)&server->addr, sizeof(server->addr));
    if (cnct < 0){
        SysPrint(RED, true, "Failed to Connect to Server. Error %i", errno);
        return;
    }

    localClient->addressInfo = server->addr;
    localClient->cfd = cfd;
    localClient->connectedServer = server;

    printf("about to enter chat room: %s\n", localClient->handle);
    User dereferenceClient = *localClient;

    // Send client info to server you're joining
    int sentBytes = send(cfd, (void*)&dereferenceClient, sizeof(dereferenceClient), 0);
    
    // TODO: HANDLE SITUATIONS THAT SEND BREAKS

    // Receive most updated server info
    Server updatedServer = {0};
    int recvBytes = recv(cfd, (void*)&updatedServer, sizeof(Server), 0);
    localClient->connectedServer = &updatedServer;

    Chatroom(&updatedServer);
}

void* JoinServerByName(void* name){ // Join server from its alias
    UpdateServerList();
    Server* server = ServerFromAlias((char*)name);
    if (!server) { // ServerFromAlias returns null if no server is found
        SysPrint(RED, true, "No Server Found With That Name.");
        return;
    }

    JoinServer(server);
}

void* JoinServerByListIndex(int index) {

    // Check if its a valid index
    // Less than 0
    if (index < 0)
        return;

    // Greater than the amount of servers online
    if (index >= onlineServers)
        return;

    // Valid index, join server
    Server* server = &serverList[index];
    JoinServer(server);
}



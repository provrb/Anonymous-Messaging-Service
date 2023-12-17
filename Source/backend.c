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

#include "Headers/backend.h"
#include "Headers/browser.h"
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief           Get a Server struct from a server alias(name)
 * @param[in]       alias: Name of the server to get info on
 * @return          Server
 * @retval          Struct of info about the server
 */
Server* ServerFromAlias(char* alias) {
    bool duplicateNames = false;
    int foundServers = 0;
    Server* sarr[] = {};
    Server* foundServer = NULL;

    for (int i=0; i < onlineServers; i++){
        Server* server = &serverList[i];

        if (server->alias != NULL){
            toLowerCase(alias); 
            toLowerCase(server->alias);

            // Compare everything lowercased
            if (strcmp(server->alias, alias) == 0)
            {
                sarr[foundServers] = server;
                foundServers+=1;
                foundServer = server;
            }
        }
    }

    if (foundServers > 1)
    {
        // Ask user which to choose
        SysPrint(UNDR, true, "Choose Between These %i Servers", foundServers);
        duplicateNames = true;
        Server* server = NULL;

        for (int i = 1; i < foundServers + 1; i++)
        {
            server = sarr[i];
            printf("%i: [%i/%i] Host: %s - Server Name: %s\n", i, server->connectedClients, server->maxClients, server->host.handle, server->alias);
        }

        while (1)
        {
            char option[10];

            printf("Option: ");

            fgets(option, 10, stdin);
            
            // Remove the trailing newline character if it exists
            if (option[strlen(option) - 1] == '\n') {
                option[strlen(option) - 1] = '\0';
            }

            if (atoi(option) <= foundServers)
            {
                foundServer = sarr[atoi(option)];
                

                break;
            }
        }
    }

    return foundServer;
}

bool IsValidFileDescriptor(unsigned int fileDescriptor)
{
    return (fcntl(fileDescriptor, F_GETFD) != -1);
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

    /*
        Pass dereference local client to the server
        we are joining to ask them to add us to the
        client list and return updated information
        about the server were connecting to
    */
    User dereferenceClient = *localClient;
    dereferenceClient.cfd = cfd;
    dereferenceClient.addressInfo = server->addr;
    dereferenceClient.connectedServer = server;

    // Send client info to server you're joining
    int sentBytes = send(cfd, (void*)&dereferenceClient, sizeof(dereferenceClient), 0);
    if (sentBytes <= 0)
    {
        SysPrint(RED, true, "Error Sending Client Info To Server. Error %i", errno);
        return;
    }

    // Receive most updated server info
    Server updatedServer = {0};
    int recvBytes = recv(cfd, (void*)&updatedServer, sizeof(Server), 0);
    if (recvBytes <= 0)
    {
        SysPrint(RED, true, "Error Receiving Client Info From Server. Error %i", errno);
        return;
    }

    /*
        Update localClient struct.
    */
    localClient->addressInfo = server->addr;
    localClient->cfd = cfd;
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



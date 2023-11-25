/**
 * ****************************(C) COPYRIGHT {2023} Blue Bear****************************
 * @file       root.c
 * @brief      functions which control the root server
 * 
 * @note       
 * @history:
 *   Version   Date            Author          Modification    Email
 *   V1.0.0    Oct-17-2023     Ethan Oliveira                  ethanjamesoliveira@gmail.com
 * 
 * @verbatim
 * ==============================================================================
 * 
 * ==============================================================================
 * @endverbatim
 * ****************************(C) COPYRIGHT {2023} Blue Bear****************************
 */
#ifndef __ROOT_H_
#define __ROOT_H_

#include "server.h"
#include "backend.h"
#include "browser.h"

#define ROOT_PORT 18081 // Port the root server is running on

extern int    onlineGlobalClients; // online world-wide clients
extern User   rootConnectedClients[]; // clients connected to the root server
extern Server rootServer; // Root server that holds all servers and handles all client connections

ResponseCode DoRootRequest(void* request);

void* AcceptClientRoot();

void* PerformRequestsFromClient(void* usr);

// Create root server all clients connect to
int CreateRootServer();

// Connect to the root server to receive world wide updates
int ConnectToRootServer(); // Root server will always be on port 43832

void DisconnectClientFromRootServer(User* user); 

void DisconnectClientFromServer(User* user);

// Respond to a clients request
void Respond(User* to, RootResponse response);

// Update a client info in the connected server
void UpdateClientInConnectedServer(User* userToUpdate);

// Update a server in serverList with new info/the updated Server struct
void UpdateServerWithNewInfo(Server* server);

#endif
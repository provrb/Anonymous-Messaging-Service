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

#include "server.h"
#include "backend.h"
#include "browser.h"

#define ROOT_PORT 18081 // Port the root server is running on

rcodes DoRequest(void* request);

void* AcceptClientRoot();
void* RecvClientRoot(void* usr);

// Create root server all clients connect to
int CreateRootServer();

// Connect to the root server to receive world wide updates
int ConnectToRootServer(); // Root server will always be on port 43832

// A client joined the root server
void ClientJoinedRoot();

// Disconnect the client server-sided
void DisconnectClientRoot(); 

void Respond(User* to, RootResponse response);
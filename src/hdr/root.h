/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
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
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 */
#ifndef __ROOT_H__
#define __ROOT_H__

#include "server.h"
#include "backend.h"
#include "browser.h"

/*
    THe port that the root server
    will be running on. 
    
    All clients who open the app 
    will connect to this port
*/
#define ROOT_PORT 18081 // Port the root server is running on

/*
    An integer of the total online clients
    that are on the app and connected to the root server
*/
extern unsigned int onlineGlobalClients;

/*
    Array of all clients who are connected
    to the root server.
*/
extern User rootConnectedClients[];

/*
    Server structure representing the root
    server information.
*/
extern Server rootServer;

/*
    Do a request made from a client on the root server.

    For example, make a server. 
    Respond to the client who made the request
    with a ResponseCode indicating error or success.
*/
ResponseCode DoRootRequest(void* request);

/*
    Accept client to join the root server

    Receive the clients information on join using recv
    and update it. Finally send back the update client
    struct to them.
*/
void* AcceptClientsToRoot();

/*
    Perform request made to the root server from the client

    Forever do this unless an error happens
    and the loop breaks. Made in a thread.
    Do this for every client connected to the root server.
    
    TODO: Resource heavy so going to try another way. But this works for now
*/
void* PerformRootRequestFromClient(void* client);

/*
    Create a root server which all clients connect to.

    This root server is only created once on the 
    server machine. All client instances must
    connect to this server to proceed in the app.
*/
int CreateRootServer();

/*
    Remove a client from the root server server-sided.

    Remove them from rootConnectedClients array,
    decrement onlineGlobalClients by one,
    and shutdown any servers the user made.
*/
void DisconnectClientFromRootServer(User* user); 

/*
    Disconnect 'user' from the server in connectedServer field.

    If the user is the host of the server,
    shut it down and disconnect other clients
    Otherwise, update the statistics of the server
    like connectedClients appropriately
*/
void DisconnectClientFromServer(User* user);

/*
    Respond to a client who made a root request.

    Send a struct 'RootResponse' to give information
    or return values on their request that happened server-sided.
*/
void RespondToRootRequestMaker(User* to, RootResponse response);

/*
    Update a server in serverList with 'updatedServerInfo'

    Gets the index of the server by searching names in serverList
    and replaces the 'Server' struct at that index with 'updatedServerInfo'
*/
void UpdateServerWithNewInfo(Server* updatedServerInfo);

#endif // __ROOT_H__
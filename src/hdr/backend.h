/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 * @file       backend.h
 * @brief      control all backend operations for the interface
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

#ifndef __BACKEND__
#define __BACKEND__

#include <string.h>          
#include <stdio.h>
#include <ctype.h>
#include "cli.h"
#include "server.h"
#include "tools.h"
#include "client.h"

/*
    Get a server struct from a server name

    If the server does not exist, NULL will be returned.
    Otherwise, a pointer to that 'Server' struct will be returned.
*/
Server* ServerFromAlias(char* alias);

/* 
    Make a request to the root server.

    The request is sent over a tcp socket by sending a 'RootRequest'
    struct that includes everything about the request.
    This function fills in the struct with the arguments provided.
    
    Once the root server receives the request, it will try and 
    perform it. After it will return a struct called 'RootResponse'
    that includes information about what happened on the root server.
*/
RootResponse MakeRootRequest(
    CommandFlag    commandFlag,
    Server    currentServer,
    User      relatedClient, 
    CMessage clientMessageInfo
); 

/*
    Exit the client application.

    Safely removes the client and makes a request
    to remove the client from any servers and update
    the statistics on the root server. i.e online clients.
*/
void ExitApp();

/* Removed */
// void EnableDebugMode();

// ------ Join Server Functions --------
/* 
    A couple different ways for a user to join a server.
    All of them achieve the same goal.
*/
void*  JoinServer(Server* server); // Join a server from 'Server' struct
void* JoinServerByName(void* name); // Join server from servers alias
void* JoinServerByListIndex(int index); // Join server from index in the server list

#endif // __BACKEND__
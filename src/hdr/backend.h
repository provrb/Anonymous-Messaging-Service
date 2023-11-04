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
#include "server.h"
#include "tools.h"

typedef void (*CommandFunction)();

// Get Server Struct from server name
Server* ServerFromAlias(char* alias);

// Make a request TO the root server
RootResponse MakeRootRequest(
    cflags    commandFlag,
    dflags    dataFlag,
    Server    currentServer,
    User      relatedClient, 
    CMessage* clientMessageInfo
); 

CommandFunction ExitApp();

/**
 *    --------[ SETTINGS FUNCTIONS ]--------
 * Functions to control the settings
 * for the user. 
 * 
 * Settings reset every session.
 * 
 */
CommandFunction EnableDebugMode();

/**
 * 
 *      --------[ JOIN FUNCTIONS ]--------         
 * Many different ways a user can join a server
 * Used with commands and arguements.
 * 
 */
CommandFunction* JoinServerByName(void* name); // Join server from its alias
CommandFunction* JoinServerByListIndex(int index); // Join server from index in the server list

#endif // __BACKEND__
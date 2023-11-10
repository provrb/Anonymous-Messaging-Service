/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 * @file       browser.h
 * @brief      server browser controller
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

#ifndef __BROWSER__
#define __BROWSER__

#include <stdio.h>
#include <stdbool.h>
#include "ccmds.h"
#include "cli.h"
#include "server.h"
#include "ccolors.h"

extern unsigned int onlineServers; // Number of online servers
extern Server       serverList[kMaxServersOnline]; // List of all online servers

// Add a server to serverList
int AddServerToList(Server server);

// Set the current serverList to the most updated serverList
Server* UpdateServerList(); 

#endif // __BROWSER__
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
#include "server.h"
#include "ccmds.h"
#include "cli.h"
#include "ccolors.h"

extern unsigned int onlineServers; // Number of online servers
extern Server       serverList[kMaxServersOnline]; // List of all online servers

/*
    Add a server to the server list.

    Once the server is added to the server list
    it will be viewable by clients and joinable.
    All information about the server will be copied
    and saved into the list.
*/
int AddServerToList(Server server);

/*
    Update the server list client-side.

    Make a request to the root server asking for
    the most updated server list that is currently
    saved on the root application.

    The server list is updated automatically and returned.
*/
Server* UpdateServerList(); 

#endif // __BROWSER__
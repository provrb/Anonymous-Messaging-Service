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

#ifndef __BROWSER_H__
#define __BROWSER_H__

#include <stdio.h>
#include <stdbool.h>
#include "server.h"
#include "ccmds.h"
#include "cli.h"
#include "ccolors.h"

/*
    The number of currently online servers
    in 'serverList'
*/
extern unsigned int onlineServers;

/*
    An array of all servers that are currently online.

    Offline servers are removed from
    this array automatically.
*/
extern Server       serverList[kMaxServersOnline];

/*
    A hashmap of all servers created.

    Used to find if a server is similiar to
    a different one. For example, trying to
    make a server you can check 'serverHashmap' and see
    if a server is already made with the same info as requested
    in O(1) time-complexity.
*/
extern Server       serverHashmap[kMaxServersOnline];

/*
    Update the server list client-side.

    Make a request to the root server asking for
    the most updated server list that is currently
    saved on the root application.

    The server list is updated automatically and returned.
*/
Server* UpdateServerList(); 

#endif // __BROWSER_H__
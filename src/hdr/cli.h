/**
 * ****************************(C) COPYRIGHT {2023} Blue Bear****************************
 * @file       cli.h
 * @brief      all things related to the command line interface 
 * 
 * @note       
 * @history:
 *   Version   Date            Author          Modification    Email
 *   V1.0.0    Nov-02-2023     Ethan Oliveira                  ethanjamesoliveira@gmail.com
 * 
 * @verbatim
 * ==============================================================================
 * 
 * ==============================================================================
 * @endverbatim
 * ****************************(C) COPYRIGHT {2023} Blue Bear****************************
 */

#ifndef _CLI_H_
#define _CLI_H_

#include <string.h>          
#include <stdio.h>
#include <ctype.h>

#include "server.h"
#include "tools.h"
#include "root.h"
#include "client.h"

void* UI(); // Main user interface

int SplashScreen(); // Main Mnnu
int DisplayCommands(); // shown by using --help cmd
int DisplayServers(); // show all servers
int DisplayServerInfo(char* serverName); // display information about a server
int TotalOnlineServers(); // Display how many online servers there are

int Chatroom(Server* server); // Chatroom for server

// AMS system message which looks like "[AMS] Msg"
void SysPrint(
    const char* color,
    bool prefixNewline, 
    const char* str, 
    ...
); 

#endif // _CLI_H_
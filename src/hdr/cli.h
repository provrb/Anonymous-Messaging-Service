/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
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
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 */

#ifndef __CLI_H__
#define __CLI_H__

#include <string.h>          
#include <stdio.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include "tools.h"
#include "root.h"
#include "server.h"

/*
    Has the clients ui been loaded/printed yet?
    Make sure it ui is not printed more than once.
    
    Prevents CSLoadClientUserInterface() from being ran
    multiple times and thus printing the ui more than once.
*/
extern bool userInterfaceLoaded;

/*
    Initialize the client user interface.

    Allows the client to input text into the terminal/console,
    displays the splash screen when they load into the app, and
    shows the command.

    Functions should be ran only once.
*/
void CSLoadClientUserInterface(); // Main user interface

/*
    Main menu of the application. 
    Greets the user and shows a couple neat points/tips.
*/
void CSSplashScreen();

/*
    Display commands the user can perform and type.
    Prints all 'Command' structs from validCommands array.
*/
void CSDisplayCommands();

/*
    Display all the online servers.

    Shows the servers index in 'serverList', 
    name, clients online and max allowed clients.
*/
void CSDisplayServers();

/*
    Display all the information about a server.

    Takes in a server name and prints info about
    it if the server exists. Otherwise print an error message.
*/
void CSDisplayServerInfo(char* serverName);

/*
    Print an integer representing how many servers are online.
*/
void CSTotalOnlineServers();

/*
    Print an interface for the client to send messages
    in a server to other clients. Takes input from the client.

    Prints out messages sent by other clients.
*/
int CSServerChatroom(Server* server); // CSServerChatroom for server

/*
    System messages that can be printed with
    a color in the terminal. 
    Includes a prefix that looks like "[AMS] Your Message..."
*/
void SysPrint(
    const char* color,
    bool prefixNewline, 
    const char* str, 
    ...
); 

/*
    Clear any output in the terminal/console.
    Works cross-platform (linux and windows);
*/
void ClearOutput();

#endif // __CLI_H__
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

#ifndef __CLI_H_
#define __CLI_H_

#include <string.h>          
#include <stdio.h>
#include <ctype.h>
#include <termios.h>
#include "unistd.h"

#include "tools.h"
#include "root.h"
#include "server.h"

/*
    Has the clients ui been loaded/printed yet?
    Make sure it doesn't be printed more than once.
    
    Prevents LoadClientUserInterface() from being ran
    multiple times.
*/
extern bool userInterfaceLoaded;

/*
    Initialize the client user interface.

    Allows the client to input text into the terminal/console,
    displays the splash screen when they load into the app, and
    shows the command.

    Functions should be ran only once.
*/
void LoadClientUserInterface(); // Main user interface

/*
    Main menu of the application. 
    Greets the user and shows a couple neat points/tips.
*/
void SplashScreen();

/*
    Display commands the user can perform and type.
    Prints all 'Command' structs from validCommands array.
*/
void DisplayCommands();

/*
    Display all the online servers.
    Shows the index in 'serverList', name, clients online and max allowed clients.
*/
void DisplayServers();

/*
    Display all the information about a server.
    Takes in a server name and prints info about
    it if the server exists. Otherwise print an error message.
*/
void DisplayServerInfo(char* serverName);

/*
    Print an integar representing how many servers are online.
*/
void TotalOnlineServers();

/*
    Print an interface for the client to send messages
    in a server to other clients.

    Prints out messages sent by other clients.
*/
int Chatroom(Server* server); // Chatroom for server

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



#endif // _CLI_H_
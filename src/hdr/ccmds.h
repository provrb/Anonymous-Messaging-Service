/**
 * ****************************(C) COPYRIGHT {2023} Blue Bear****************************
 * @file       ccmds.h
 * @brief      Client commands used in the cli
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

#ifndef __INTERFACE_H_
#define __INTERFACE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "backend.h"

/*
    Specific functions that can be ran
    through the command-line by the client.
    
    These command functions do NOT have parameters.
*/
typedef void (*CommandFunction)();

/*
    A structure representing a typeable command
    for the client.

    A client can enter 'commandName' to perform a command,
    provided that the arguments are all entered correctly.
    
    If 'function' is null, it means the function takes
    arguments and they cannot be passed through a simple
    'CommandFunction'' function like in the struct below.
*/
typedef struct CMD {
    const char* commandName;
    char* cmdDesc;
    CommandFunction function;    
} Command; 

extern Command   validCommands[]; // A list of valid command structs that can be ran by client in cli
extern const int numOfCommands; // The number of valid commands

#endif // __INTERFACE_H_
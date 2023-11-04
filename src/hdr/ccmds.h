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

// Turn int preproccessor definition to string
#define STRINGIFY(x) #x 

// Represents a command with a function to execute the command
typedef struct CMD {
    const char* commandName;
    char* cmdDesc;
    CommandFunction function;    
} Command; 

extern Command validCommands[];
extern const int numOfCommands;

#endif // __INTERFACE_H_
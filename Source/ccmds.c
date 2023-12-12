/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
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
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 */

#include "Headers/ccmds.h"
#include "Headers/client.h"
#include "Headers/server.h"
#include "Headers/cli.h"

// Map of valid commands
Command validCommands[] = {
    {"--help"                                    , "Show list of commands"            , DisplayCommands},
    {"--servers"                                 , "Show all servers"                 , DisplayServers}, // Show list of servers
    {"--so"                                      , "Number of online servers"         , TotalOnlineServers},
    {"--si <server-name>"                        , "View Info Of a Server"            , NULL},
    {"--main"                                    , "Show The Main Menu"               , SplashScreen},
    // {"--dbg"                                     , "Toggle Debug mode"                , EnableDebugMode},
    {"--quit"                                    , "Exit The Application"             , ExitApp},
    {"--joins <server-name>"                     , "Join Server With The Name"        , NULL},
    {"--makes <server-name> <port> <max-clients>", "Make a Server With Specified Name", NULL}
};

const int kNumOfCommands = sizeof(validCommands) / sizeof(validCommands[0]);
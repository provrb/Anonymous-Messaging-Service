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

#include "hdr/ccmds.h"
#include "hdr/client.h"
#include "hdr/server.h"
#include "hdr/cli.h"

// Map of valid commands
Command validCommands[] = {
    {"--help"                                    , "Show list of commands"            , CSDisplayCommands},
    {"--servers"                                 , "Show all servers"                 , CSDisplayServers}, // Show list of servers
    {"--so"                                      , "Number of online servers"         , CSTotalOnlineServers},
    {"--si <server-name>"                        , "View Info Of a Server"            , NULL},
    {"--main"                                    , "Show The Main Menu"               , CSSplashScreen},
    // {"--dbg"                                     , "Toggle Debug mode"                , EnableDebugMode},
    {"--quit"                                    , "Exit The Application"             , CSExitAMS},
    {"--joins <server-name>"                     , "Join Server With The Name"        , NULL},
    {"--makes <server-name> <port> <max-clients>", "Make a Server With Specified Name", NULL}
};

const int kNumOfCommands = sizeof(validCommands) / sizeof(validCommands[0]);
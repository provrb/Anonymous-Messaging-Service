/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 * @file       browser.c
 * @brief      server browser manager
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

#include "hdr/browser.h"

unsigned int onlineServers = 0;
Server       serverList[MAX_ALLOWED_SERVERS] = {0};

/**
 * @brief           Add a valid server to server list
 * @param[in]       server: server to add
 * @return          int
 * @retval          success or failure
 */
int AddServerToList(Server server){
    if (server.online == false){
        printf("ERROR: Cannot add offline server to list");
        return -1;
    }

    serverList[onlineServers] = server;
    onlineServers+=1;

    return 0;
}


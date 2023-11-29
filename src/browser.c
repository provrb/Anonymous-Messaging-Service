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
Server       serverList[kMaxServersOnline] = {0};
Server       serverHashmap[kMaxServersOnline] = {0};

Server* UpdateServerList() {
    RootResponse response = CSMakeRootRequest(
        k_cfRequestServerList,
        (Server){0}, // no related server
        (User){0},  // no user
        (CMessage){0} // No cmessage
        );
    
    if (response.rcode != k_rcRootOperationSuccessful)
        return NULL;
    
    // CSMakeRootRequest already updates serverList
    // and onlineServers client side
    // When k_cfRequestServerList is passed.
    return serverList;
}

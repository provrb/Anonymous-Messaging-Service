/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 * @file       root.c
 * @brief      functions which control the root server. NOTE: ALL OF THESE FUNCTIONS SHOULD BE SERVER-SIDED
 * 
 * @note       
 * @history:
 *   Version   Date            Author          Modification    Email
 *   V1.0.0    Oct-17-2023     Ethan Oliveira                  ethanjamesoliveira@gmail.com
 * 
 * @verbatim
 * ==============================================================================
 * 
 * ==============================================================================
 * @endverbatim
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 */

#include "Headers/root.h"

/*
    Global statistics about the
    root server
*/
unsigned int onlineGlobalClients = 0; // All clients connected to the root server
User   rootConnectedClients[] = {};  // List of all clients on the root server
Server rootServer = { 0 };          // Root server info

void SSUpdateClientWithNewInfo(User updatedUserInfo)
{
    // Iterate through rootConnectedClients list to find the client to update
    for (size_t i = 0; i < onlineGlobalClients; i++)
    {
        if (strcmp(rootConnectedClients[i].handle, updatedUserInfo.handle) == 0) 
        {
            rootConnectedClients[i] = updatedUserInfo;
            break;
        }
    }
}

void RSUpdateServerWithNewInfo(Server* updatedServerInfo)
{
    // Iterate through server list to find the server to update
    for (size_t i = 0; i < onlineServers; i++)
    {
        // 'server' is at index 'i' in the list.
        if (strcmp(serverList[i].alias, updatedServerInfo->alias) == 0) 
        {
            // found it now replace it with updated server aka 'server'
            serverList[i] = *updatedServerInfo;
            break;
        }
    }
    // Now the server should be updated in the serverList
}

void UpdateClientInConnectedServer(User* userToUpdate)
{
    if (!userToUpdate->connectedServer->online)
        return;

    Server* server = userToUpdate->connectedServer;

    // Get the index of the client from the connectedServers Client list
    int clientIndexInCList = ClientIndex(
                                server->clientList,
                                server->connectedClients,
                                userToUpdate);
    
    // Temp array to copy all clients to including the updated client info
    User** tmpArray[] = {0};
    
    // Copy over the connected servers client list to tmpArray
    // and input the most updated info into tmpArray
    for (int i=0; i<server->connectedClients; i++)
    {
        // Found the user to update in the connected servers client list
        if (strcmp(server->clientList[i]->handle, userToUpdate->handle) == 0)
            // Put updated User* struct in tmpArray
            tmpArray[i] = userToUpdate;
        else
            // Normal client we dont need to update
            tmpArray[i] = server->clientList[i];
    }

    // Now copy tmpArray into connected servers client list so client list
    // is fully up to date
    for (int i=0; i<server->connectedClients; i++)
        server->clientList[i] = tmpArray[i];

    // Finally take changes into affect and update this server on the root server
    RSUpdateServerWithNewInfo(server);
}

/**
 * @brief           Make a request from the client to the root server. Kind of like http
 * @param[in]       commandFlag:   tell the server what to do with the data
 * @param[in]       currentServer: server to make the request from
 * @param[in]       relatedClient: client who made the request
 * @return          void*
 * @retval          RootResponse from server as void*
 */
RootResponse MakeRootRequest(
    CommandFlag    commandFlag,
    Server    currentServer,
    User      relatedClient,
    CMessage clientMessageInfo
)
{
    RootRequest request;
    request.cmdFlag           = commandFlag;
    request.server            = currentServer;
    request.user              = relatedClient;
    request.clientSentMessage = clientMessageInfo;

    // Default response values
    RootResponse response = {0};
    response.rcode        = k_rcInternalServerError;
    response.rflag        = k_rfNoResponse;
    response.returnValue  = NULL;

    // Send request to root server
    int sentBytes = send(rootServer.sfd, (const void*)&request, sizeof(RootRequest), 0);
    if (sentBytes <= 0) { // Client disconnected or something went wrong sending
        printf(RED "Error making request to root server...\n" RESET);
        return response;
    }

    // Client wont be able to receive messages when their socket file descriptor is closed
    if (request.cmdFlag == k_cfDisconnectClientFromRoot)
        return response;

    // Receive a response from the root server
    int receivedBytes = recv(rootServer.sfd, (void*)&response, sizeof(RootResponse), 0);
    if (receivedBytes < 0) { // Client disconnected or something went wrong receiving
        printf(RED "Failed to receive data from root server...\n" RESET);
        return response;
    }
 
    // In the case of special commands
    // where we may need to send or recv more than once
    switch (request.cmdFlag)
    {
    case k_cfMakeNewServer:
        break;
    case k_cfRequestServerList: // Need to receive all servers individually
    {
        // Receive the number of online servers
        uint32_t onlineServersTemp = 0;
        int initialReceivedBytes = recv(rootServer.sfd, &onlineServersTemp, sizeof(onlineServersTemp), 0);
        
        if (initialReceivedBytes <= 0)
            break;
        
        onlineServers = ntohl(onlineServersTemp);

        // Receive all servers
        for (int i = 0; i < onlineServers; i++){
            Server receivedServer = {0};
            int receive = recv(rootServer.sfd, (void*)&receivedServer, sizeof(Server), 0);
            
            if (receive <= 0)
                break;
            
            // Update server list
            serverList[i] = receivedServer;
        }

        break;
    }
    default:
        break;
    }

    return response;
}

ResponseCode DoRootRequest(void* req)
{
    RootRequest  request = *(RootRequest*)req;
    RootResponse response;
    memset(&response, 0, sizeof(response)); 

    pthread_t pid;

    // Default response
    response.rcode       = k_rcRootOperationSuccessful;
    response.returnValue = NULL;
    response.rflag       = k_rfSentDataWasUnused;

    // Handle possible request commands
    switch (request.cmdFlag)
    {
    case k_cfRequestServerList: // Client wants to know the updated server list 
        RSRespondToRootRequestMaker(&request.user, response);

        // First send the amount of online servers as an int
        uint32_t nlOnlineServers = htonl(onlineServers); // htonl version of onlineServers int
        
        int sentBytes = send(request.user.rfd, &nlOnlineServers, sizeof(nlOnlineServers), 0);
        if (sentBytes <= 0)
            SysPrint(RED, true, "Error sending online servers int. Errno %i", errno);

        // After prepare and send all the servers from the list individually
        for (int servIndex=0; servIndex<onlineServers; servIndex++)
        {
            Server server = serverList[servIndex];
            send(request.user.rfd, (void*)&server, sizeof(Server), 0);
        }

        break;
    case k_cfAppendServer: // Add server to server list
        SSAddServerToList(request.server);
        response.rflag = k_rfNoValueReturnedFromRequest;
        RSRespondToRootRequestMaker(&request.user, response);
        break;
    case k_cfRemoveServer: // Remove server from server list
        int index = -1;
        for (int i=0; i < onlineServers; i++){
            if (strcmp(serverList[i].alias, request.server.alias) == 0) // Server exists in the list
                index = i;
        }

        if (index == -1) // Server doesnt exist...
            break;

        // shift array to remove that server
        for (int b=index; b < onlineServers - 1; b++) 
            serverList[b] = serverList[b + 1]; 
        
        onlineServers--;
        // Server list now removed
        RSRespondToRootRequestMaker(&request.user, response);
        break;
    case k_cfMakeNewServer: // Make new server and run it
        // Index will be the port hashed by max servers allowed online
        int indexInList = request.server.port % kMaxServersOnline;

        // Make sure port isnt in use
        // Try and find the port in the hash map
        // Check if ports in use
        if (portList[indexInList].port == request.server.port && portList[indexInList].inUse)
        {
            // in use
            response.rcode = k_rcErrorPortInUse;
            response.returnValue = (void*)request.server.port;
            response.rflag = k_rfSentDataWasUnused;
            RSRespondToRootRequestMaker(&request.user, response);
            break;
        }

        // Add it to the list of ports
        PortDesc portInfo = {0};
        portInfo.inUse = true;
        portInfo.port = request.server.port;
        portList[indexInList] = portInfo;

        ServerCreationInfo creationInfo = {0};
        creationInfo.serverInfo = &request.server;
        creationInfo.clientAKAhost = &request.user;

        if (pthread_create(&pid, NULL, RSServerBareMetal, (void*)&creationInfo) != 0)
            SysPrint(RED, true, "***** FATAL ERROR: Failed to Make Thread for Server. Errno %i. Aborting.", errno);

        break;
    // pthread_exit(NULL);
    case k_cfDisconnectClientFromRoot:
        RSDisconnectClientFromRootServer(request.user);
        return k_rcRootOperationSuccessful;
    case k_cfKickClientFromServer:
        SSDisconnectClientFromServer(&request.user);
        break;
    case k_cfRSUpdateServerWithNewInfo:
        RSUpdateServerWithNewInfo(&request.server);
        break;
    case k_cfSSUpdateClientWithNewInfo:
        request.user.connectedServer = &request.server;
        SSUpdateClientWithNewInfo(request.user);
        response.rflag = k_rfRequestedDataUpdated;
        RSRespondToRootRequestMaker(&request.user, response);
        break;
    default:
        return k_rcInternalServerError; // LIkely the command doesnt exist
    }

    return response.rcode;
}
 
void* AcceptClientsToRoot() {
    pthread_t clientThreads[kMaxGlobalClients]; // all active threads
    int activeClientThreads = 0; // The number of active client threads
    Server rootServerBackup = rootServer;
    
    while (1) {
        // Make sure onlineGlobalClients does not exceed max allowed clients
        if (onlineGlobalClients + 1 >= kMaxGlobalClients)
            continue;

        int cfd = accept(rootServerBackup.sfd, (struct sockaddr*)NULL, NULL);

        if (cfd < 0) // Bad client file descriptor.
            continue;

        // Receive client info on join
        RootRequest request = { 0 };
        int clientInfo = recv(cfd, (void*)&request, sizeof(request), 0);
        
        if (clientInfo < 0) 
        {
            SysPrint(RED, false, "Error Receiving Client '%i' Info.", cfd);
            close(cfd);
            continue;
        }

        // user wants to join
        if (request.cmdFlag == k_cfConnectClientToServer)
        {
            SysPrint(CYN, false, "%s Joined!", request.user.handle);
                        
            request.user.rfd             = cfd;
            request.user.connectedServer = &rootServer;
            rootConnectedClients[onlineGlobalClients] = request.user;
            onlineGlobalClients++;

            // Send info back
            RootResponse response = {0};
            response.rcode        = k_rcRootOperationSuccessful;
            response.returnValue  = (void*)&request.user;
            response.rflag        = k_rfRequestedDataUpdated;
           
            int sendResponse = send(cfd, (void*)&response, sizeof(response), 0);
            if (sendResponse <= 0)
            {
                printf(RED "\tError Sending Updated Struct Back\n" RESET);
                continue;
            }

            // Add thread to all activeClientThreads list
            pthread_t thread;
            clientThreads[activeClientThreads] = thread;

            // Make a thread for each client who connects
            if (pthread_create(&clientThreads[activeClientThreads], NULL, PerformRootRequestFromClient, (void*)&request.user) != 0)
            {
                printf("Failed to create thread for client\n");
                continue;
            }
            
            rootServer = rootServerBackup;

            activeClientThreads++;
        }
    }
    pthread_exit(NULL);
}

void* PerformRootRequestFromClient(void* client) {
    
    User* connectedClient = (User*)client;
    
    /*
        Forever receive requests
        from a client unless a condition is met
        to break from the loop. Usually an error.
    */
    while (1) 
    {
        RootRequest receivedRequest = { 0 };
        size_t      receivedBytes = recv(connectedClient->rfd, (void*)&receivedRequest, sizeof(RootRequest), 0);
        
        if (receivedBytes < 0) 
        {
            SysPrint(RED, false, "Failed to Receive Message");
            continue;
        }
        else if (receivedBytes == 0)
        {
            RSDisconnectClientFromRootServer(receivedRequest.user);
            break;
        }

        // No command to perform
        if (receivedRequest.cmdFlag == k_cfNone)
            continue;

        if (receivedRequest.cmdFlag != k_cfDisconnectClientFromRoot)
            receivedRequest.user = *connectedClient;
        else
        {
            receivedRequest.user.connectedServer = &receivedRequest.server;
            // disconnect user
            RSDisconnectClientFromRootServer(receivedRequest.user);
            break;
        }
        
        ResponseCode result = DoRootRequest((void*)&receivedRequest);
        if (receivedRequest.cmdFlag == k_cfDisconnectClientFromRoot)
        {
            break;
        }

        if (result != k_rcRootOperationSuccessful) 
        {
            printf(RED "Error Doing Request '%i' From %s\n" RESET, receivedRequest.cmdFlag, connectedClient->handle);
            continue;
        }
    }

    pthread_exit(NULL);
} 

void RSRespondToRootRequestMaker(User* to, RootResponse response) {
    fprintf(stderr, CYN "[AMS] Response to Client '%s' ", to->handle);
    int snd = sendto(to->rfd, (void*)&response, sizeof(response), MSG_NOSIGNAL, (struct sockaddr*)&to->addressInfo, sizeof(to->addressInfo));
    
    if (snd <= 0)
        printf(RED "Failed\n" RESET);
    else
        printf(GRN "Good\n" RESET);
}

void RSDisconnectClientFromRootServer(User usr) {
    int index = 0;

    close(usr.cfd);
    close(usr.rfd);

    // Get index where client is on the root connected clients arra    
    for (int i = 0; i < onlineGlobalClients; i++){
        // Is the user we want equal to the user in the list at the index 'i'
        if (strcmp(rootConnectedClients[i].handle, usr.handle) == 0)
            index = i;
    }

    // remove client from rootConnectedClients by shifting array
    for (int i = index; i < onlineGlobalClients - 1; i++) 
        rootConnectedClients[i] = rootConnectedClients[i + 1]; 


    // Check if client is connected to server
    // If client is, update the server statistics
    if (!usr.connectedServer->isRoot) {
        if (IsUserHost(usr, usr.connectedServer))
        {
            ShutdownServer(usr.connectedServer);
        }
    }

    // Remove 1 client from connected client count
    onlineGlobalClients--;
}

/**
 * @brief           Create a root server all clients connect to when starting the app
 * @return          int
 * @retval         successful result
 */
int CreateRootServer() {
    // DEBUG_PRINT("Creating root server. Make sure this is only made once");
    printf(BLU "\t|||| INITIAL ROOT SERVER CREATION ||||\n" RESET);
    printf("Creating root server. Filling in struct... ");

    rootServer.port       = ROOT_PORT;
    rootServer.domain     = AF_INET;
    rootServer.type       = SOCK_STREAM;
    rootServer.protocol   = 0;
    rootServer.maxClients = -1; // Infinite connections
    rootServer.isRoot     = true;
    strcpy(rootServer.alias, "__root__");

    printf("Done\n");

    printf("Creating socket for the server... ");
	int sfd = socket(rootServer.domain, rootServer.type, rootServer.protocol);
    if (sfd < 0)
        return -1;
    printf("Done\n");

    const bool kReuseAddr = true;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &kReuseAddr, sizeof(int));

    printf("Creating server address info... ");
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family      = rootServer.domain;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port        = htons(rootServer.port);

    printf("Done\n");
    printf("Binding socket to address info... ");

	int bnd = bind(sfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (bnd < 0) {
        close(sfd);
        return -1;
    }

    printf("Done\n");
    printf("Setting up listener for client connections... ");

	int lsn = listen(sfd, 10);
    if (lsn < 0) {
        close(sfd);
        return -1;
    }

    printf("Done\n");

    rootServer.addr = serv_addr; 
    rootServer.sfd  = sfd;

    printf(BLU "\t     |||| ROOT SERVER CREATED ||||\n" RESET);

    return 0;
}


void SSDisconnectClientFromServer(User* user) {
    Server* server = user->connectedServer;

    // get index of client in server client list
    int index = -1;
    for (int i=0; i < server->connectedClients; i++){
        if (strcmp(server->clientList[i]->handle, user->handle) == 0) // Server exists in the list
            index = i;
    }            

    // shift array to remove client from clientlist
    for (int b=index; b < server->connectedClients-1; b++) 
        server->clientList[b] = server->clientList[b + 1]; 

    server->connectedClients--;

    close(user->cfd);
    
    // Update server with new info since 
    // the client list and connectedClients has been updated
    RSUpdateServerWithNewInfo(server);

    if (IsUserHost(*user, server))
    {
        ShutdownServer(server);
    }

    user->connectedServer = &rootServer;
    printf("Now the user is connected to %s\n", user->connectedServer->alias);
    SSUpdateClientWithNewInfo(*user);

}
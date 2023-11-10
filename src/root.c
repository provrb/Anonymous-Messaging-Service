/**
 * ****************************(C) COPYRIGHT {2023} Blue Bear****************************
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
 * ****************************(C) COPYRIGHT {2023} Blue Bear****************************
 */

#include "hdr/root.h"

void UpdateServerWithNewInfo(Server* server)
{
    // Iterate through server list to find the server to update
    for (size_t i = 0; i < onlineServers; i++)
    {
        // 'server' is at index 'i' in the list.
        if (strcmp(serverList[i].alias, server->alias) == 0) 
        {
            // found it now replace it with updated server aka 'server'
            serverList[i] = *server;
            break;
        }
    }

    // Now the server should be updated in the serverList
}

void UpdateClientInConnectedServer(User* userToUpdate)
{
    if (!userToUpdate->connectedServer->online)
        return NULL;

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
    UpdateServerWithNewInfo(server);
}

ResponseCode DoRequest(void* req)
{
    RootRequest request = *(RootRequest*)req;
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
        Respond(&request.user, response);

        // First send the amount of online servers as an int
        uint32_t nlOnlineServers = htonl(onlineServers); // htonl version of onlineServers int
        
        int sentBytes = send(request.user.rfd, &nlOnlineServers, sizeof(nlOnlineServers), 0);
        if (sentBytes <= 0)
            SysPrint(RED, true, "Error sending online servers int. Errno %i", errno);

        // After prepare and send all the servers from the list individually
        for (int servIndex=0; servIndex<onlineServers; servIndex++)
        {
            Server server = serverList[servIndex];
            printf("\t[%i] - [%i/%i]: %s\n", servIndex+1, server.connectedClients, server.maxClients, server.alias);
            printf("\t\tHost: %s\n", server.host->handle); 
            send(request.user.rfd, (void*)&server, sizeof(Server), 0);
        }

        break;
    case k_cfAppendServer: // Add server to server list
        AddServerToList(request.server);
        response.rflag = k_rfNoValueReturnedFromRequest;
        Respond(&request.user, response);
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
        for(int b=index; b < onlineServers - 1; b++) 
            serverList[b] = serverList[b + 1]; 
        
        onlineServers--;
        // Server list now removed
        Respond(&request.user, response);
        break;
    case k_cfMakeNewServer: // Make new server and run it
        ServerCreationInfo creationInfo = {0};
        creationInfo.serverInfo = &request.server;
        creationInfo.clientAKAhost = &request.user;

        if (pthread_create(&pid, NULL, ServerBareMetal, (void*)&creationInfo) != 0)
            SysPrint(RED, true, "***** FATAL ERROR: Failed to Make Thread for Server. Errno %i. Aborting.", errno);
        break;
    case k_cfDisconnectClientFromRoot:
        DisconnectClientFromRootServer(&request.user);
        break;
    // TODO: Handle situation server sided
    case k_cfKickClientFromServer:
        DisconnectClientFromServer(&request.user);
        break;
    case k_cfUpdateClientInfoInServer:
        UpdateClientInConnectedServer(&request.user);
        break;
    default:
        return k_rcInternalServerError; // LIkely the command doesnt exist
    }

    return response.rcode;
}
 
void* AcceptClientRoot() {
    int cfd = -1;
    pthread_t clientThreads[kMaxGlobalClients]; // all active threads
    int activeClientThreads = 0; // The number of active client threads

    while (1) {
        cfd = accept(rootServer.sfd, (struct sockaddr*)NULL, NULL);
        if (onlineGlobalClients + 1 >= kMaxGlobalClients || cfd < 0)
            continue;

        // Receive client info on join
        RootRequest request;
        memset(&request, 0, sizeof(RootRequest));
        int clientInfo = recv(cfd, (void*)&request, sizeof(request), 0);
        if (clientInfo < 0) printf("Error receiving client info.\n");

        // user wants to join
        if (request.cmdFlag == k_cfConnectClientToServer) {
            printf(GRN "\n%s Came Online!\n" RESET, request.user.handle);
            
            
            request.user.rfd                      = cfd;
            request.user.connectedServer          = &rootServer;
            rootConnectedClients[onlineGlobalClients] = request.user;
            onlineGlobalClients++;

            // Send info back
            RootResponse response = {0};
            response.rcode        = k_rcRootOperationSuccessful;
            response.returnValue  = (void*)&request.user;
            response.rflag        = k_rfRequestedDataUpdated;
           
            int sendResponse = send(cfd, (const void*)&response, sizeof(response), 0);
            if (sendResponse <= 0) {
                printf(RED "\tError Sending Updated Struct Back\n" RESET);
                continue;
            }

            // Add thread to all activeClientThreads list
            pthread_t thread;
            clientThreads[activeClientThreads] = thread;

            // Make a thread for each client who connects
            if (pthread_create(&clientThreads[activeClientThreads], NULL, RecvClientRoot, (void*)&request.user) != 0){
                printf("Failed to create thread for client\n");
                continue;
            }

            pthread_join(clientThreads[activeClientThreads], NULL);

            activeClientThreads++;
        }
    }
}

void* RecvClientRoot(void* usr) {
    User* connectedClient = (User*)usr;

    // loop will be created for every client
    // to handle requests from each client
    while (1) {
        RootRequest receivedInfo;
        size_t res = recv(connectedClient->rfd, (void*)&receivedInfo, sizeof(RootRequest), 0);
        if (res < 0) {
            SysPrint(RED, true, "Failed to Receive Message");
            continue;
        } else if (res == 0) {
            SysPrint(WHT, true, "Client '%s' Disconnected from the Server", connectedClient->handle);
            DisconnectClientFromRootServer(connectedClient);
            break;
        }

        // No command to perform
        if (receivedInfo.cmdFlag == k_cfNone || receivedInfo.cmdFlag == 0)
            continue;

        // ReceivedInfo User should always be connectedCLient
        if (strcmp(receivedInfo.user.handle, connectedClient->handle) != 0)
            // Not the same user in request as connectedClient
            continue;

        ResponseCode result = DoRequest((void*)&receivedInfo);

        if (result != k_rcRootOperationSuccessful) {
            printf(RED "Error Doing Request '%i' From %s\n" RESET, receivedInfo.cmdFlag, connectedClient->handle);
            break;
        }
    }
    pthread_exit(NULL);
}

void Respond(User* to, RootResponse response) {
    printf("Sending Response to Client %s With RFD %i.\n", to->handle, to->rfd);
    int snd = sendto(to->rfd, (void*)&response, sizeof(response), 0, (struct sockaddr*)&to->caddr, sizeof(to->caddr));
    
    if (snd == -1 || snd <= 0) {
        perror("Send failed");
    } else {
        printf("Sent\n");
    }
}

void DisconnectClientFromRootServer(User* usr) {
    int index = 0;

    // Get index where client is on the root connected clients arra    
    for (int i = 0; i < onlineGlobalClients; i++){
        // Is the user we want equal to the user in the list at the index 'i'
        if (strcmp(rootConnectedClients[i].handle, usr->handle) == 0)
            index = i;
    }

    // remove client from rootConnectedClients by shifting array
    for(int i = index; i < onlineGlobalClients - 1; i++) 
        rootConnectedClients[i] = rootConnectedClients[i + 1]; 


    // Check if client is connected to server
    // If client is, update the server statistics
    if (usr->connectedServer->online) {
        // Get index of client where it is on the connected servers client list array
        for (int i = 0; i < usr->connectedServer->connectedClients; i++){
            if (strcmp(usr->handle, usr->connectedServer->clientList[i]->handle) == 0)
                // REMOVE CLIENT FROM THE SERVERS CLIENT LIST
                index = i;
        }

        // remove client from connected server client list by shifting array
        // Decrease connected clients 
        for(int i = index; i < usr->connectedServer->connectedClients - 1; i++) 
            usr->connectedServer->clientList[i] = usr->connectedServer->clientList[i + 1]; 

        if (IsUserHost(usr, usr->connectedServer))
            ShutdownServer(usr->connectedServer->alias);
    }

    // Remove 1 client from connected client count
    onlineGlobalClients--;
}

/**
 * @brief           Connect to the server which holds information about all other servers
 * @return          int
 * @retval          success
 */
int ConnectToRootServer() {
    MallocClient();

    // Fill out information about the root server used to establish client connection.

    printf("Filling root server struct... ");
    
    rootServer.domain     = AF_INET;
    rootServer.type       = SOCK_STREAM;
    rootServer.protocol   = 0;
    rootServer.port       = ROOT_PORT;
    rootServer.maxClients = -1;
    rootServer.isRoot     = true;
    strcpy(rootServer.alias, "__root__");

    printf("Done\n");
    printf("Creating client socket using port %i... ", rootServer.port);

    // Connect client to server. Establish a connection

    int cfd = socket(rootServer.domain, rootServer.type, rootServer.protocol); // These are the settings the root server uses
    if (cfd < 0)
        goto close_root_connection;

    printf("Done\n");
    printf("Filling address info for client to connect... ");

    // Address info for main server

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = rootServer.domain;
    addr.sin_port   = htons(rootServer.port); // Constant. Server will always be on this port

    // For now it will run on local machine

    if (inet_pton(rootServer.domain, "127.0.0.1", &addr.sin_addr) < 0)
        goto close_root_connection;
    
    printf("Done\n");
    fprintf(stderr, "Connecting client socket to main server... ");
    // Attempt to retry connection again if it fails.

    int attempts = 0;
    do {
        int con = connect(cfd, (struct sockaddr*)&addr, sizeof(addr));
        if (con < 0) {
            attempts++;
            sleep(2);
            continue;
        }
        break;
    }
    while (attempts < 5);
    if (attempts >= 5) goto close_root_connection;

    printf("Done\n");
    printf("Client connected to main server.\n");
    printf("Filling out local client info struct... ");
   
    // Fill out local client info struct
    // Handle already filled out at start of program
    client->cfd             = 0;
    client->removeMe        = false;
    client->caddr           = addr;
    client->connectedServer = NULL;
    client->rfd             = cfd;
    client->joined = gmt();
    AssignDefaultHandle(client->handle);
    rootServer.sfd = cfd;

    printf("Done\n");
    
    // Send client info
    RootRequest req = {0};
    req.clientSentMessage = (CMessage){0};
    req.cmdFlag = k_cfConnectClientToServer;
    req.server = rootServer;
    req.user = *client;
    int sendInfo = send(rootServer.sfd, (const void*)&req, sizeof(req), 0);

    printf("Sent current user info. Now trying to recv.\n");

    // Get response. Updated user info
    RootResponse resp = {0};
    int recvInfo = recv(rootServer.sfd, (void*)&resp, sizeof(resp), 0);

    printf("Received updated user info.\n");

    if (resp.rcode == k_rcRootOperationSuccessful && resp.rflag == k_rfRequestedDataUpdated)
    {
        User* updatedClient = (User*)&resp.returnValue;
        client = updatedClient;
    }

    pthread_t pid;
    if (pthread_create(&pid, NULL, ClientJoinedRoot, NULL) != 0){
        // DEBUG_PRINT("Error making client handler thread");
        goto close_root_connection;
    }

    pthread_join(pid, NULL);

    // Success
    return 0;

// Error
close_root_connection:
    // DEBUG_PRINT("Closing root connection for client");
    printf("Failed\n");
    printf(RED "Failed to connect to main servers. Error code %i, %i attempts\n" RESET, errno, attempts);
    close(cfd);
    return -1;
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
    AcceptClientRoot();

    return 0;
}

void ClientJoinedRoot() {
    ChooseClientHandle();
    UI();
}

void DisconnectClientFromServer(User* user) {
    if (user->connectedServer->online)
    {
        // get index of client in server client list
        int index = -1;
        for (int i=0; i < user->connectedServer->connectedClients; i++){
            if (strcmp(user->connectedServer->clientList[i]->handle, user->handle) == 0) // Server exists in the list
                index = i;
        }            

        // shift array to remove client from clientlist
        for(int b=index; b < user->connectedServer->connectedClients - 1; b++) 
            user->connectedServer->clientList[b] = user->connectedServer->clientList[b + 1]; 

        user->connectedServer->connectedClients--;

        close(user->cfd);
        
        // Update server with new info since 
        // the client list and connectedClients has been updated
        UpdateServerWithNewInfo(user->connectedServer);

        if (IsUserHost(user, user->connectedServer))
            ShutdownServer(user->connectedServer->alias);
    }
}
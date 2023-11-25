
#include "hdr/client.h"
#include "hdr/ccolors.h"

User*  localClient = {0}; // Current client who ran the app

void MallocClient() {
    localClient = (User*)malloc(sizeof(User));
}

void DecryptPeerMessage(CMessage* message)
{
    for (int charIndex = 0; charIndex<strlen(message->message); charIndex++)
    {
        message->message[charIndex] = message->message[charIndex] ^ message->cflag;
    }
}


ResponseCode MakeServerRequest(
    CommandFlag command,
    User        requestMaker,
    CMessage    optionalClientMessage,
    Server      server
)
{
    ServerRequest request         = {0};
    request.command               = command;
    request.optionalClientMessage = optionalClientMessage;
    request.requestMaker          = requestMaker;

    ResponseCode response;

    int sentBytes = send(requestMaker.cfd, (void*)&request, sizeof(request), 0);
    // todo: handle situation.. too lazy

    // Don't received on connected server socket if we left the server
    if (request.command != k_cfKickClientFromServer)
    {
        int recvBytes = recv(optionalClientMessage.sender->cfd, (void*)&response, sizeof(ResponseCode), 0);
        // todo: handle situation.. too lazy        
    }
    else 
    {
        // Client left the server so remove connectedServer
        localClient->connectedServer = (Server*){0};
        localClient->cfd = 0;
    }

    return response;
}

size_t ClientIndex(const User* arr[], size_t size, User* value)
{
    for (int i=0; i<size; i++)
    {
        if (strcmp(arr[i]->handle, value->handle) == 0)
            return i;
    }

    return -1;
}

void DisconnectClient(){
    // Remove client from root connected clients

    SysPrint(WHT, true, "Disconnecting %s", localClient->handle);
    localClient->removeMe = true;
    RootResponse response = MakeRootRequest(k_cfDisconnectClientFromRoot, rootServer, *localClient, (CMessage){0});
    if (response.rcode = k_rcRootOperationSuccessful){
        SysPrint(GRN, false, "Disconnected. Goodbye");
    }

    exit(EXIT_SUCCESS);
}

void ReceiveMessageFromServer(void* serverInfo)
{
    Server* server = (Server*)serverInfo;
    while (1)
    {
        CMessage receivedCMessage = {0};
        int receivedBytes = recv(localClient->cfd, (void*)&receivedCMessage, sizeof(receivedCMessage), 0);

        DecryptPeerMessage(&receivedCMessage);

        // todo: handle situation
        if (receivedBytes <= 0)
            continue;

        switch (receivedCMessage.cflag)
        {
        case k_cfUpdateServerWithNewINfo:
            
            break;
        case k_cfPrintEchodClientMessage:
            printf("%s: %s\n", receivedCMessage.sender->handle, receivedCMessage.message);
            break;
        // TODO: Handle situation
        case k_cfKickClientFromServer:
            printf("You have been kicked from '%s'...\n", server->alias);
            MakeRootRequest(k_cfKickClientFromServer, *server, *localClient, (CMessage){0});
            break;
        case k_cfBanClientFromServer:
            break;
        default:
            break;
        }
    }
    pthread_exit(NULL);
}

/**
 * @brief           Handle clientinput by the user and treat it as a app command
 * @return          void*
 * @retval          Nothing.
 */
void* HandleClientInput(){
    while (1) {
        char cmd[100];
        bool validCmd = false;

        printf("Enter Command> ");

        fgets(cmd, kMaxCommandLength, stdin);

        // Remove the trailing newline character if it exists
        if (cmd[strlen(cmd) - 1] == '\n') {
            cmd[strlen(cmd) - 1] = '\0';
        }

        // Command is empty, don't need to process anything
        if (strlen(cmd) == 0) {
            continue;
        }

        /*
        *
        * All functions that take command line arguemnts
        * In a case a function takes a command line argument,
        * it must be processed specially like below
        * 
        */

        // Server info command. It takes command-line arguments
        if (strstr(cmd, "--si") != NULL) {
            char serverName[kMaxServerAliasLength + 1];

            if (sscanf(cmd, "--si %32s", serverName) != 1) {
                SysPrint(RED, true, "Invalid Usage for --si. View --help for more info.");
                continue;
            }

            DisplayServerInfo(serverName);
            validCmd = true;
        }

        // Join server command. It takes command-line arguments
        else if (strstr(cmd, "--joins") != NULL) {
            char serverName[kMaxServerAliasLength + 1];

            if (sscanf(cmd, "--joins %32s", serverName) != 1) {
                SysPrint(RED, true, "Invalid Usage for --joins. View --help for more info.");
                continue;
            }

            printf("sounds good: %s\n", localClient->handle);
            JoinServerByName(serverName);
            validCmd = true;
        }

        else if (strstr(cmd, "--makes") != NULL) {
            char         serverName[kMaxServerAliasLength + 1];
            unsigned int maxClients;
            unsigned int port;

            if (sscanf(cmd, "--makes %32s %u %u", serverName, &port, &maxClients) != 3) {
                SysPrint(RED, true, "Invalid Usage for --makes. View --help for more info.");
                continue;
            }

            SysPrint(CYN, true, "Making Server '%s' on Port '%i'", serverName, port);

            int serverCreated = MakeServer(AF_INET, SOCK_STREAM, 0, port, maxClients, serverName);
            validCmd = true;
        }

        // Normal command function without command-line args
        for (int i = 0; i < numOfCommands; i++)
        {
            if (strcmp(cmd, validCommands[i].commandName) == 0) {
                validCommands[i].function();
                validCmd = true;
                break;
            }
        }

        if (!validCmd)
        {
            SysPrint(RED, true, "Unknown Command. Enter --help to view commands.");
            continue;
        }
    }

    pthread_exit(NULL);
}

/**
 * @brief           Choose your username throughout the stay on the app
 * @retval          Nothing
 */
void ChooseClientHandle() {
    bool warned = false;
    bool goodUsername = false; // Does the client username inputted follow the rules

    SysPrint(CYN, true, "Enter a username before joining:");
    
    // Make sure username is valid and follows rules
    do {
        char username[kMaxClientHandleLength + 1];                   
        printf(CYN "Select Your Username> " RESET);        
        fgets(username, kMaxClientHandleLength, stdin);    
        
        // Remove the trailing newline character if it exists
        if (username[strlen(username) - 1] == '\n') {
            username[strlen(username) - 1] = '\0';
        }
        
        for (int i=0; i < numOfCommands; i++){
            if (strstr(username, validCommands[i].commandName) != NULL) 
            {
                SysPrint(YEL, true, "[WARNING]: Username cannot be a command.");
                warned = true;
                break;
            }
        }

        if (warned) {
            warned=false;
            continue;
        }

        if (strlen(username) <= 2 && !warned){
            SysPrint(YEL, true, "[WARNING]: Username too short (Min 3 Chars).");
            warned=!warned;
            continue;
        }

        goodUsername=true;
        strcpy(localClient->handle, username);
    } while(!goodUsername);

    SysPrint(CYN, false, "You will now be known as: '%s' for this session.", localClient->handle);
}

void AssignDefaultHandle(char defaultName[]) {
    time_t now = time(NULL);
    struct tm* timestr = gmtime(&now);

    char nameIdentifier[kMaxClientHandleLength + 1];
    sprintf(nameIdentifier, "%d", timestr->tm_sec);

    strcpy(defaultName, "usr");
    strcat(defaultName, nameIdentifier);
}

int DefaultClientConnectionInfo() {
    localClient->removeMe = false;
    localClient->joined = gmt();
    localClient->caddr = rootServer.addr;
    localClient->cfd = 0;
    localClient->rfd = rootServer.sfd;
    return 0;
}
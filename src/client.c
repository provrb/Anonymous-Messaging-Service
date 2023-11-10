#include "hdr/client.h"
#include "hdr/ccolors.h"

void MallocClient() {
    client = (User*)malloc(sizeof(User));
}

size_t ClientIndex(const User* arr[], size_t size, User* value)
{
    for (int i=0; i<size; i++)
    {
        if (strcmp(arr[i]->handle, value->handle) == 0)
            return i;
    }

    return -1;
    // size_t index = 0;

    // while ( index < size && arr[index] != value ) ++index;

    // return ( index == size ? -1 : index );
}

void DisconnectClient(){
    // Remove client from root connected clients

    SysPrint(WHT, true, "Disconnecting %s", client->handle);
    client->removeMe = true;
    RootResponse response = MakeRootRequest(k_cfDisconnectClientFromRoot, rootServer, *client, (CMessage){0});
    if (response.rcode = k_rcRootOperationSuccessful){
        SysPrint(GRN, false, "Disconnected. Goodbye");
    }

    exit(EXIT_SUCCESS);
}

void RecvClientMessages(void* serverInfo)
{
    Server* server = (Server*)serverInfo;
    while (1)
    {
        CMessage receivedCMessage = {0};
        int receivedBytes = recv(client->cfd, (void*)&receivedCMessage, sizeof(receivedCMessage), 0);

        // todo: handle situation
        if (receivedBytes <= 0)
            continue;

        switch (receivedCMessage.cflag)
        {
        case k_cfPrintEchodClientMessage:
            printf("%s: %s\n", receivedCMessage.sender->handle, receivedCMessage.message);
            break;
 
        // TODO: Handle situation
        case k_cfKickClientFromServer:
            printf("You have been kicked from '%s'...\n", server->alias);
            MakeRootRequest(k_cfKickClientFromServer, *server, *client, (CMessage){0});
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
 * @brief           Handle input by the user and treat it as a app command
 * @return          void*
 * @retval          Nothing.
 */
void* HandleClientInput(){
    bool prnt = true;
    while (1) {
        char cmd[100];
        bool successCmd = false;

        if (prnt) {
            printf("Enter Command> ");
            prnt = false;
        }

        fgets(cmd, kMaxCommandLength, stdin);

        if (strlen(cmd) == 0) {
            continue; // Skip processing if the command is empty
        }

        printf("You inputted: %s\n", cmd);

        // Find command function
        for (int i = 0; i < numOfCommands; i++) {
            // Server info command. It takes command-line arguments
            if (strstr(cmd, "--si") != NULL) {
                char serverName[kMaxServerAliasLength + 1];

                if (sscanf(cmd, "--si %32s", serverName) != 1) {
                    SysPrint(RED, true, "Invalid Usage for --si. View --help for more info.");
                    break;
                }

                DisplayServerInfo(serverName);

                successCmd = true;
                break;
            }

            // Join server command. It takes command-line arguments
            if (strstr(cmd, "--joins") != NULL) {
                char serverName[kMaxServerAliasLength + 1];

                if (sscanf(cmd, "--joins %32s", serverName) != 1) {
                    SysPrint(RED, true, "Invalid Usage for --joins. View --help for more info.");
                    break;
                }

                JoinServerByName(serverName);

                successCmd = true;
                break;
            }

            if (strstr(cmd, "--makes") != NULL) {
                char serverName[kMaxServerAliasLength + 1];
                unsigned int maxClients;
                unsigned int port;

                if (sscanf(cmd, "--makes %32s %u %u", serverName, &port, &maxClients) != 3) {
                    SysPrint(RED, true, "Invalid Usage for --makes. View --help for more info.");
                    break;
                }

                printf("OK. Making Server '%s' on Port %i. Max clients : %i\n", serverName, port, maxClients);

                int serverCreated = MakeServer(AF_INET, SOCK_STREAM, 0, port, maxClients, serverName);
                successCmd = true;
                break;
            }

            // Normal command function without command-line args
            if (strcmp(cmd, validCommands[i].commandName) == 0) {
                validCommands[i].function();
                successCmd = true;
                break;
            }
        }
        if (successCmd) prnt = true;

        if (!successCmd) {
            SysPrint(RED, true, "Unknown Command. Enter --help to view commands.");
            prnt = true;
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
        strcpy(client->handle, username);
    } while(!goodUsername);

    SysPrint(CYN, false, "You will now be known as: '%s' for this session.", client->handle);
}

void AssignDefaultHandle(char defaultName[]) {
    time_t now = time(NULL);
    struct tm* timestr = gmtime(&now);

    char nameIdentifier[kMaxClientHandleLength + 1];
    sprintf(nameIdentifier, "%d", timestr->tm_sec);

    strcpy(defaultName, "usr");
    strcat(defaultName, nameIdentifier);
}
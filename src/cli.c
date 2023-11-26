/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 * @file       cli.c
 * @brief      all things regarding command line interface for the user
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

#include "hdr/cli.h"
#include "hdr/client.h"


/*
    By default, the user interface is not laoded.
    Changed when LoadClientUserInterface is called
*/
bool userInterfaceLoaded = false;


/**
 * @brief           Prints to the console with a prefix. Wrapper for printf
 * @param[in]       color: color of the text to print
 * @param[in]       prefixNewline: whether to add a newline at the beginning
 * @param[in]       str: string to print
 * @param[in]       ...: formatting
 * @retval          void
 */
void SysPrint(const char* color, bool prefixNewline, const char* str, ...) { 
    va_list argp;
    va_start(argp, str);
    
    // If you want a newline before all the text
    if (prefixNewline)
        printf("\n");
    
    printf("%s[AMS] ", color);

    // Print the message and variable args
    vfprintf(stdout, str, argp);
    va_end(argp);    
    printf(RESET "\n");
}


/**
 * @brief           Main interface function, runs everything front end
 * @return          void*: Needed for threading
 * @retval          Does not return anything.
 */
void LoadClientUserInterface() {
    if (!userInterfaceLoaded)
    {
        userInterfaceLoaded = true;

        DefaultClientConnectionInfo();
        SplashScreen();

        DisplayCommands(); // Display commands when you load up
        
        // Create a thread to handle client inpuit
        pthread_t cmd_tid;
        if (pthread_create(&cmd_tid, NULL, HandleClientInput, NULL) != 0){
            SysPrint(RED, true, "*** ERROR CREATING HandleClientInput() THREAD. EXITING APPLICATION. ***");
            ExitApp();
        }

        pthread_join(cmd_tid, NULL);
        pthread_exit(&cmd_tid);
    }
}

/**
 * @brief           Splash screen with text output to the console
 * @return          int
 * @retval          represents success
 */
void SplashScreen() {
    struct tm* timestr = gmt();
    printf(CYN "\nWelcome to AMS! The Time Is: %02d:%02d:%02d UTC\n", timestr->tm_hour, timestr->tm_min, timestr->tm_sec);
    printf("  - Search and create your own servers.\n");
    printf("  - Chat completely anonymously.\n");
    printf("  - No saved data. All chat logs deleted. \n" RESET);
}

/**
 * @brief           Shows all online servers
 * @return          int
 * @retval          success
 */
void DisplayServers() {

    UpdateServerList();

    // Print server list header
    SysPrint(UNDR WHT, true, "Server List (%i Online):", onlineServers);
    printf("  [ID] - [USR/COUNT] : SERVER NAME\n");

    // Print Server List
    for (int servIndex=0; servIndex<onlineServers; servIndex++){
        Server server = serverList[servIndex];
        printf("\t[%i] - [%i/%i]: %s\n", servIndex+1, server.connectedClients, server.maxClients, server.alias);
    }
}

/**
 * @brief           Display all application commands
 * @return          int
 * @retval          represents success
 */
void DisplayCommands() {
    int maxCommandLength = 0;

    // Find the maximum length of kCommandName to align the command names
    for (int i = 0; i < kNumOfCommands; i++) {
        int commandLength = strlen(validCommands[i].kCommandName);
        if (commandLength > maxCommandLength) {
            maxCommandLength = commandLength;
        }
    }

    SysPrint(UNDR WHT, true, "> Showing All (%d) Commands:", kNumOfCommands);

    // Display commands with aligned colons
    for (int i = 0; i < kNumOfCommands; i++) {
        printf("\t%-*s : %s\n", maxCommandLength, validCommands[i].kCommandName, validCommands[i].cmdDesc);
    }
}

void ClearOutput()
{
#ifdef _WIN64
    system("cls")
#elif __linux__
    system("clear");
#endif
}

void DisableTerminalInput() {
    system("stty -echo");
}

void EnableTerminalInput() {
    system("stty echo");
}

int Chatroom(Server* server) {

    // Clear previous terminal output
    ClearOutput();
    SERVER_PRINT(CYN, "You are now connected to '%s'", server->alias);
    SERVER_PRINT(CYN, "Use '--leave' to Disconnect.\n");

    // Create thread to print other client messages to the screen
    pthread_t tid;
    if (pthread_create(&tid, NULL, ReceivePeerMessagesOnServer, (void*)server) < 0) {
        printf("Internal server error. Aborting.\n");
        return -1;
    }

    /*
        Take input from the local client.

        Send input as a message to all other clients in the server
        unless it is a command. i.e --leave; don't see '--leave' to the other clients.
    */
    while (1)
    {
        EnableTerminalInput();
        char* message = malloc(kMaxClientMessageLength);
        if (message == NULL)
            break;

        printf("msg> "); // message prompt
        fgets(message, kMaxClientMessageLength, stdin);
        DisableTerminalInput();

        // Remove the trailing newline character if it exists
        if (message[strlen(message) - 1] == '\n') {
            message[strlen(message) - 1] = '\0';
        }

        if (strlen(message) < 0)
            continue;

        if (strcmp(message, "--leave") == 0)
        {
            ResponseCode leaveRequest = MakeServerRequest(k_cfKickClientFromServer, *localClient, (CMessage){0});
            break;
        }

        CMessage cmsg = {0};
        cmsg.cflag    = k_cfEchoClientMessageInServer;
        cmsg.sender   = localClient;
        strcpy(cmsg.message, message);
        
        ResponseCode requestStatus = MakeServerRequest(k_cfEchoClientMessageInServer, *localClient, cmsg);
        
        printf("\x1b[1A");
        printf("\x1b[2K");
        printf("\r");
        
        if (requestStatus == k_rcInternalServerError)
            printf(RED "Failed to Send Message.\n" RESET);
        else
            printf("%s: %s\n", localClient->handle, cmsg.message);
        
        sleep(1);
    }

    ClearOutput();
    SysPrint(CYN, false, "Disconnected From the Server '%s'", server->alias);
    SplashScreen();
    return 0;
}

/**
 * @brief           Display information about a server
 * @param[in]       serverName: Name of the server to get info about
 * @return          int
 * @retval          Success code
 */
void DisplayServerInfo(char* serverName) {
    UpdateServerList();
    Server* server = ServerFromAlias(serverName);
    
    if (server == NULL) 
    {
        SysPrint(RED, true, "No Server Found With Name '%s'", serverName);
    }
    else
    {
        SysPrint(UNDR WHT, true, "Found Information on Server '%s'. Displaying:", serverName);
        printf("\tDomain            : %i\n", server->domain);
        printf("\tType              : %i\n", server->type);
        printf("\tProtocol          : %i\n", server->protocol);
        printf("\tPort              : %i\n", server->port);
        printf("\tConnected Clients : %i\n", server->connectedClients);
        printf("\tMax Clients       : %i (%i/%i)\n", server->maxClients, server->connectedClients, server->maxClients);
        printf("\tServer Alias/Name : %s\n", server->alias);        
    }
}

/**
 * @brief           Simply print the amount of online servers
 * @return          int
 * @retval          0
 */
void TotalOnlineServers() {
    UpdateServerList();
    SysPrint(WHT UNDR, true, "There Are %i Online Servers", onlineServers);
}


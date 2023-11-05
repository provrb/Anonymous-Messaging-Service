/**
 * ****************************(C) COPYRIGHT {2023} Blue Bear****************************
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
 * ****************************(C) COPYRIGHT {2023} Blue Bear****************************
 */

#include "hdr/cli.h"

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
void* UI() {
    SplashScreen();
    pthread_t cmd_tid;

    DisplayCommands(); // Display commands when you load up
    
    // Create a thread to handle client inpuit
    if (pthread_create(&cmd_tid, NULL, HandleClientInput, NULL) != 0){
        SysPrint(RED, true, "*** ERROR CREATING HandleClientInput() THREAD. EXITING APPLICATION. ***");
        exit(EXIT_FAILURE);
    }

    pthread_join(cmd_tid, NULL);
    pthread_exit(&cmd_tid);
}

/**
 * @brief           Splash screen with text output to the console
 * @return          int
 * @retval          represents success
 */
int SplashScreen() {
    struct tm* timestr = gmt();
    printf(CYN "\nWelcome to AMS! The Time Is: %02d:%02d:%02d UTC\n", timestr->tm_hour, timestr->tm_min, timestr->tm_sec);
    printf("  - Search and create your own servers.\n");
    printf("  - Chat completely anonymously.\n");
    printf("  - No saved data. All chat logs deleted. \n" RESET);

    return 0;
}

/**
 * @brief           Shows all online servers
 * @return          int
 * @retval          success
 */
int DisplayServers() {

    // Make request to servers for server list
    RootResponse returnInfo = MakeRootRequest(cf_REQUEST_SERVER_LIST, df_NONE, rootServer, *client, NULL); // Request server list

    if (returnInfo.rcode != rc_SUCCESSFUL_OPERATION) 
        return -1;

    // Print server list header
    SysPrint(UNDR WHT, true, "> Server List (%i Online):", onlineServers);
    printf("\t[ID] : [USR/COUNT] : SERVER NAME\n");

    // Print Server List
    for (int servIndex=0; servIndex<onlineServers; servIndex++){
        Server server = serverList[servIndex];
        printf("\t[%i] - [%i/%i]: %s\n", servIndex+1, server.connectedClients, server.maxClients, server.alias);
    }

    return 0;
}

/**
 * @brief           Display all application commands
 * @return          int
 * @retval          represents success
 */
int DisplayCommands() {
    int maxCommandLength = 0;

    // Find the maximum length of commandName to align the command names
    for (int i = 0; i < numOfCommands; i++) {
        int commandLength = strlen(validCommands[i].commandName);
        if (commandLength > maxCommandLength) {
            maxCommandLength = commandLength;
        }
    }

    SysPrint(UNDR WHT, true, "> Showing All (%d) Commands:", numOfCommands);

    // Display commands with aligned colons
    for (int i = 0; i < numOfCommands; i++) {
        printf("\t%-*s : %s\n", maxCommandLength, validCommands[i].commandName, validCommands[i].cmdDesc);
    }
            
    return 0;
}

int Chatroom(Server* server) {

    // Clear previous terminal output
#ifdef _WIN64
    system("cls")
#elif __linux__
    system("clear");
#endif
    
    // Create thread to print other client messages to the screen
    pthread_t tid;
    if (pthread_create(&tid, NULL, RecvClientMessages, (void*)server) < 0) {
        printf("Internal server error. Aborting.\n");
        return -1;
    }

    // Send messages
    bool sentMessage = false;
    while (1)
    {
        char* message[MAX_MSG_LEN + 1];
        printf("%s:msg> ", client->handle); // message prompt with username
        scanf("%500s", &message);

        if (strlen(message) < 0)
            continue;

        RelayClientSentMessage(server, message, client);
    }

    return 0;
}

/**
 * @brief           Display information about a server
 * @param[in]       serverName: Name of the server to get info about
 * @return          int
 * @retval          Success code
 */
int DisplayServerInfo(char* serverName) {
    Server* server = ServerFromAlias(serverName);
    
    if (server == NULL){
        SysPrint(RED, true, "No Server Found With Name '%s'", serverName);
        return -1;
    }

    SysPrint(UNDR WHT, true, "Found Information on Server '%s'. Displaying:", serverName);
    
    printf("\tDomain            : %i\n", server->domain);
    printf("\tType              : %i\n", server->type);
    printf("\tProtocol          : %i\n", server->protocol);
    printf("\tPort              : %i\n", server->port);
    printf("\tConnected Clients : %i\n", server->connectedClients);
    printf("\tMax Clients       : %i (%i/%i)\n", server->maxClients, server->connectedClients, server->maxClients);
    printf("\tServer Alias/Name : %s\n", server->alias);        

    return 0;
}

/**
 * @brief           Simply print the amount of online servers
 * @return          int
 * @retval          0
 */
int TotalOnlineServers() {
    SysPrint(WHT UNDR, true, "There Are %i Online Servers", onlineServers);
    return 0;
}


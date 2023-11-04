#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "hdr/server.h"
#include "hdr/client.h"

// Your server-related functions and structures here
void closeroot()
{
    // for (int i=0; i<rootConnectedClients;i++)
    //     close(rootServer.clientList[i]->cfd);

    close(rootServer.sfd);
}

int main() {
    // IMPORTANT DONT DELETE
    signal(SIGINT, closeroot);
    memset(&rootServer, 0, sizeof(rootServer));

    // First connect to the main server to view all servers
     // init client
    
    if (CreateRootServer() == -1) printf("Failed making server. errno %i\n", errno);
    printf("ran\n");
    // ConnectToRootServer(); 
    // MakeServer(AF_INET, SOCK_STREAM, 0, 8080, 10, "ethans-room");
    // ShutdownServer("ethans-room");
    // Interface();
    // MakeServer(AF_INET, SOCK_STREAM, 0, 8080, "127.0.0.1", 10, "abc", false);

    return 0;
}
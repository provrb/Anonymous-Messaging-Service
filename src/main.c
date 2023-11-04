#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "hdr/server.h"
#include "hdr/client.h"

// Your server-related functions and structures here


int main() {
    // IMPORTANT DONT DELETE
    MallocClient(); // allocate memory for client using memset
    signal(SIGINT, DisconnectClient);

    AssignDefaultHandle(client->handle);

    // First connect to the main server to view all servers
    // strcpy(client->handle, Client_AssignDefaultHandle());
    printf("Welcome to AMS, %s\n", client->handle);
    if (ConnectToRootServer() == -1) {
        printf("Aborting... Done\n"); 
        return -1;
    }
    // ShutdownServer("ethans-room");
    // MakeServer(AF_INET, SOCK_STREAM, 0, 8080, "127.0.0.1", 10, "abc", false);

    return 0;
}
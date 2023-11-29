#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "hdr/server.h"
#include "hdr/client.h"

/*
    Allocate memory and load client.
    Connect to the root server.
    Load user interface for client
*/
int main() 
{
    ClearOutput();

    /* 
        Connect interrupted terminal input (i.e Ctrl+C) 
        to CSDisconnectClient() to make sure client gets
        safely disconnected and resources are freed
    */
    signal(SIGINT, CSDisconnectClient);

    /*
        Setup base client struct.
        Allocate memory and assign a default username
    */ 
    MallocLocalClient(); 
    CSAssignDefaultHandle(localClient->handle);
    
    /* 
        Get the client to choose their username 
    */
    CSChooseClientHandle();
    
    printf("\nWelcome to AMS, %s\n", localClient->handle);

    /* 
        Connect to root server
    */
    if (CSConnectToRootServer() != 0) {
        printf("Aborting... Done\n"); 
        return -1;
    }

    /*
        Make a thread to load the client ui 
    */
    pthread_t pid;
    if (pthread_create(&pid, NULL, CSLoadClientUserInterface, NULL) != 0)
        CSDisconnectClient(localClient);

    pthread_join(pid, NULL);

    return 0;
}
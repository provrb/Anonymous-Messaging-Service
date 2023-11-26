#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "hdr/server.h"
#include "hdr/client.h"


int main() {
    /*
        Set the rootServer to all 0's
    */
    memset(&rootServer, 0, sizeof(rootServer));
    
    /*
        Create the root server on ROOT_PORT.
        Hosted on the machine thats running the 
        root application.
    */
    if (CreateRootServer() != 0) 
    {
        SysPrint(RED, false, "Failed Making Root Server. Error Code: %i\n", errno);
        return -1;
    }

    /*
        Listen and accept any client connections
        to the root server
    */
    AcceptClientsToRoot();

    return 0;
}
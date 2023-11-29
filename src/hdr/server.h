/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 * @file       server.h
 * @brief      control everything about server sockets
 * 
 * @note       
 * @history:
 *   Version   Date            Author          Modification    Email
 *   V1.0.0    Oct-06-2023     Ethan Oliveira                  ethanjamesoliveira@gmail.com
 * 
 * @verbatim
 * ==============================================================================
 * 
 * ==============================================================================
 * @endverbatim
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdbool.h>

#include "ccolors.h"
#include "flags.h"
#include "../ext/aes.h"
#include "../ext/aes-gcm.h"
#include "min_max_values.h"

// Debug mode. Allows for more printing
/** Not used **/
extern bool DEBUG;

/*
    A struct which represents a client and holds information
    about the client such as their selected username,
    file descriptors, and client address info.
*/
typedef struct UserStr
{
    char               handle[kMaxClientHandleLength + 1]; // The clients username
    struct tm*         joined;                             // Time joined CSServerChatroom | GMT Time
    unsigned int       cfd;                                // User File Descriptor (Socket)
    struct sockaddr_in addressInfo;                        // Client address info
    struct ServerStr*  connectedServer;                    // The server the client is connected to
    struct ServerStr*  ownedServer;                        // Info about the server the client created.
    unsigned int       rfd;                                // root file descriptor. Socket of the client connected to root server
    unsigned int       serversCreated;                     // How many ONLINE servers the client has created
} User;

/*
    A struct representing a server that
    clients can connect to and send requests on.
    Servers have a socket file descriptor which is used
    to send and receive information over sockets.
*/
typedef struct ServerStr 
{
    int                sfd;                              // Socket File Descriptor
    int                domain;                           // Communcation domain the server is operating on
    int                type;                             // Communcation semmantic type. All servers are SOCK_STREAM
    int                protocol;                         // Server protocol. All server protocols will be 0.
    int                port;                             // The port the server is operating on. Port must be open for it to be created.
    unsigned int       connectedClients;                 // Current # of connected clients to the server
    unsigned int       maxClients;                       // Max clients allowed to connect
    struct sockaddr_in addr;                             // address struct with info on the server address
    bool               online;                           // Bool representing whether or not the server is online
    char               alias[kMaxServerAliasLength + 1]; // used to connect to server without ip
    bool               isRoot;                           // if the connected server is the root server
    User*              host;                             // Client who requested for server to be created
    User**             clientList;                       // List of connected clients. Memory must be allocated first
    unsigned int       blacklistedClients;               // Number of clients banend from the server
    User               clientBlacklist[];                // Clients who are banned from this server.
} Server;

/*
    Info used to create a server in
    SSServerBareMetal thread. Includes the
    info to create a server and the host
    who wants to make the server.
*/
typedef struct ServerCreationInformation
{
    User*   clientAKAhost; // Host of the server. The person who requested to create the server
    Server* serverInfo;    // Info of the server to be used when creating
} ServerCreationInfo;

/*
    A struct representing a message sent from
    a client to a server. Contains a command flag
    to tell the server what to do with this message. 
*/
typedef struct ClientMessage
{
    CommandFlag cflag;                                // What to do with the message
    User*       sender;                               // Client who sent the message
    char        message[kMaxClientMessageLength + 1]; // String message
} CMessage;

/*
    A structure representing a request
    from a client to a server.

    Contains information such as what command to perform,
    and an optional 'CMessage' struct if the user wants
    to relay the message to peer clients on the server.
*/
typedef struct ClientToServerRequest
{
    CommandFlag command; // Command to tell the server to perform
    User        requestMaker; // User who is making the request
    CMessage    optionalClientMessage; // (CMessage){0} if no message
} ServerRequest;

/*
    Print out a message with the time and a [srv/] tag.
    
    Choose the color to print out as in the terminal
    Time printed out is in UTC.
    New line automatically added at the end of strings.
*/
void ServerPrint(
    const char* color,
    const char* str, 
    ... // variable args like printf("%s", --> myName <--);
);

/*
    Bare bones of creating a server.
    'Server' struct must be casted to void*
    and passed as the 'serverStruct' parameter.
    
    All fields in the struct must be completed.
    Recommended to use CSMakeServer() instead.
*/
void* SSServerBareMetal(
    void* serverStruct // Server Struct
);

/*
    Accept clients on server, 'serverInfo' indefinetly.

    Uses the 'maxClients' field to see if
    the max clients have been accepted. After
    no more clients will be accepted.
*/
void* SSAcceptClientsOnServer(void* serverInfo);

/*
    Get a server struct from a server name

    If the server does not exist, NULL will be returned.
    Otherwise, a pointer to that 'Server' struct will be returned.
*/
Server* ServerFromAlias(
    char* alias
);

/*
    Recommended way of creating a server.

    Called when the client makes a root request
    with the command flag 'k_cfMakeNewServer'
    Create a server server-sided with 
    arguments from the client who made the request
    to make a server.
*/
int CSMakeServer(
    int domain, // AF_INET
    int type, // SOCK_STREAM
    int protocol, // USUALLY ZERO
    int port,
    unsigned int maxClients,
    char* alias
);

/*
    Xor and encrypt a message in a CMessage struct.

    After calling this function, message.message should
    be encrypted and no return value is needed.
*/
void SSEncryptClientMessage(
    CMessage* message
);

/*
    Listen for any request made on
    'server' by the LATEST CONNECTED CLIENT.

    A new thread for this function is created
    for every client accepted to 'server'. This
    function listens for any requests made to the server
    by the client accepted and calls DoServerRequest()
    once a request is received.
*/
void* SSListenForRequestsOnServer(
    void* server
);

/*
    Respond to a client who made a request
    to their current connected server.

    Can only perform requests from a client
    after SSListenForRequestsOnServer() is called for that client
*/
ResponseCode DoServerRequest(
    ServerRequest request
);

/*
    Return 'true' if 'user' is the host of 'server'

    Checks the 'host' field of 'server' and compares
    the name of the host with 'user'. If they are the
    same than 'user' is the host and return true, otherwise
    return false.
*/
bool IsUserHost(
    User user,
    Server* server
);

/*
    Close a server and close all connected client sockets.

    Safely shutdown a server by closing the servers file descriptor
    as well as any client file descriptors for that server.
*/
void SSShutdownServer(
    Server* server
);

#endif // __SERVER_H__

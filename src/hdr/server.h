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

#ifndef __SERVER__
#define __SERVER__

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

// Debug mode. Allows for more printing
extern bool DEBUG;

// SERVER STATUSES
#define SERVER_FAILURE -3
#define SERVER_CREATED 9

// Max
#define NO_MORE_THAN    100 // Can't set a servers maxClient more than this
#define MAX_ALIAS_LEN   32  // Max server name length in chars
#define MAX_USRNAME_LEN 20 // Max client user name length in chars
#define MAX_MSG_LEN     500 // Max msg length in chars
#define MAX_ROOT_CLIENTS 1000 // Max allowed clients connected to app
#define MAX_ALLOWED_SERVERS 30 // Max allowed servers online

// Min
#define MIN_ALIAS_LEN   3
#define MIN_USRNAME_LEN 3

// Print only if debug is true
#define DEBUG_PRINT(str) \
  if (DEBUG == true)             \
    fprintf(stderr, "[%s:%d]: %s\n", __FILE__, __LINE__, str);

struct ServerStr; // To put it in UserStr before ServerStr is defined
struct UserStr;   // So UserStr can be compiled if it it used before defined

/**
 * @brief           Print out a message with time and server prefix
 * @param[in]       color: What to color terminal text
 * @param[in]       str: String to print
 * @param[in]       ...: Formatting
 */
static void SERVER_PRINT(const char* color, const char* str, ...);

// Information about a user
typedef struct UserStr
{
    dflags            dflag; // data_flag. See flags.h
    char              handle[MAX_USRNAME_LEN + 1]; // Username
    struct tm*        joined; // Time joined chatroom | GMT Time
    unsigned int      cfd;    // User File Descriptor (Socket)
    struct sockaddr_in caddr;  // Client address info
    struct ServerStr* connectedServer;
    bool              removeMe; // If set to true tell the server to disconnect client from root server
    int               rfd;    // root file descriptor. Socket of the client connected to root server
} User;

// Information about a server
typedef struct ServerStr 
{
    dflags             dflag; // data_flag. See flags.h
    int                sfd; // Socket File Descriptor
    int                domain;
    int                type;
    int                protocol;
    int                port;
    unsigned int       connectedClients; // Current # of connected clients to the server
    unsigned int       maxClients; // Max clients allowed to connect
    struct sockaddr_in addr; // address struct with info on the server address
    bool               online; // boolean
    char               alias[MAX_ALIAS_LEN + 1]; // used to connect to server without ip
    bool               isRoot; // if the connected server is the root server
    User*              host; // Client who requested for server to be created

    User** clientList;
} Server;

// Info to create a server with
typedef struct ServerCreationData
{
    User* clientAKAhost; // Host of the server. The person who requested to create the server
    Server* serverInfo; // Info of the server to be used when creating
} ServerCreationInfo;

// Information about a client sent message
typedef struct ClientMessage
{
    dflags dflag; // data_flag. See flags.h
    cflags cflag; // what to do with the message
    User*  sender; // Client who sent the message
    char   message[MAX_MSG_LEN + 1];
} CMessage;

// A struct with information to send the root server
// to perform a request
typedef struct CommonRecv
{
    dflags dflag;
    cflags cmdFlag; // What command to tell root server to perform
    User user;
    Server server;
    CMessage clientSentMessage; // A message sent by client. empty string if no message. ENCRYPTED
} RootRequest;

// Struct describing the result of making a root request
// Gives information on the operation- did it succeed.
typedef struct Response
{
    rflags rflag; // Response flags, tell the client what to do or what has been done
    rcodes rcode; // Response code/status code. Tell if the operation succeeded
    void*  returnValue; // Expect returned thing from making the root request like a server list
} RootResponse;

extern User*  client; // Client struct
extern bool   goodUsername; // Client username follows all rules
extern int    onlineWWClients; // online world-wide clients
extern User   rootConnectedClients[]; // clients connected to the root server
extern Server rootServer; // Root server information

// Allocate memory for the client so its usable. No segmentation faults
void MallocClient();

// Send a message sent to the server to all clients connected to that server
int RelayClientSentMessage(
    Server* server,
    char* message, 
    User* op // Original Poster/Sender
);

int SendServerMessage(
    Server* server,
    char* message
);

// Construct a server
// Use MakeServer() Rather than ServerBareMetal().
void* ServerBareMetal(
    void* serverStruct // Server Struct
);

// Wrapper for ServerBareMetal
// Creates a thread for ServerBareMetal
int MakeServer(
    int domain, // AF_INET
    int type, // SOCK_STREAM
    int protocol, // USUALLY ZERO
    int port,
    unsigned int maxClients,
    char* alias
);

// Check if 'user' is host of 'server'
bool IsUserHost(
    User* user,
    Server* server
);

// Receive messages from clients on server
void* ServerRecvThread(
    void* serverInfo
);

// Close a server and close all the client sockets connected to it
void ShutdownServer(
    char* alias
);

#endif // __SERVER__

#ifndef __CLIENT_H_
#define __CLIENT_H_

#include "server.h"
#include "browser.h"
#include "backend.h"
#include "root.h"

extern User*  localClient; // Client struct

/*
    A structure representing a request
    from a client to a server.

    Contains information such as what command to perform,
    and an optional 'CMessage' struct if the user wants
    to relay the message on the server.
*/
typedef struct ClientToServerRequest
{
    CommandFlag command;
    User        requestMaker;
    CMessage    optionalClientMessage; // (CMessage){0} if no message
} ServerRequest;

/*
    Allocate memory for the 'localClient'
*/
void   MallocClient(); 

void   DisconnectClient(); // Disconnect from the root server

size_t ClientIndex(const User* arr[], size_t size, User* value); // Get client index in array with user struct

void   ReceiveMessageFromServer(void* serverInfo); // Receive messages from other clients in the server

void   AssignDefaultHandle(char defaultName[]); // Default Client Username

void   ChooseClientHandle(); // Select your username

void*  HandleClientInput(); // handle commands entered by client

int    DefaultClientConnectionInfo();

void   DecryptPeerMessage(CMessage* message);

ResponseCode MakeServerRequest(
    CommandFlag command,
    User        requestMaker,
    CMessage    optionalClientMessage,
    Server      server
);


#endif
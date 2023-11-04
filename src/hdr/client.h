#include "server.h"
#include "browser.h"
#include "backend.h"
#include "root.h"

void   MallocClient(); 

void   DisconnectClient(); // Disconnect from the root server

size_t ClientIndex(const User* arr[], size_t size, User* value); // Get client index in array with user struct

// thread
void   RecvClientMessages(void* serverInfo); // Receive messages from other clients in the server

void   AssignDefaultHandle(char defaultName[]); // Default Client Username

void   ChooseClientHandle(); // Select your username

void*  HandleClientInput();
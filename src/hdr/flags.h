/**
 * Flags to set when transfering data over sockets.
 * Allows to know which type of structs of info you are sending         
 */
typedef enum eDataFlags
{
    df_NONE = -1, // No flags
    df_USER = 10, // is a user struct 
    df_SERVER = 20, // is a server struct
    df_CMESSAGE = 33, // is a client sent message struct
} dflags;

/**
 * Flags to set when we ant the root server to perform commands
 * Tells the root server what commands to perform with the data
 */
typedef enum cmdFlags
{
    cf_NONE = -123, // No Command

    // Commands regarding server browser/explorer
    cf_APPEND_SERVER       = 10,  // Append server to explorer 
    cf_REMOVE_SERVER       = -10, // Remove server from explorer
    cf_REQUEST_SERVER_LIST = 100, // Get all updated server list
    cf_MAKE_NEW_SERVER     = 920, // Create a new server clients can connect to

    // Commands regarding client
    cf_DISCONNECT_CLIENT_FROM_ROOT = 914, // Remove client from root server
    cf_USER_JOINED                 = 932, // User joined specifed server
    cf_BAN_CLIENT_FROM_SERVER      = 482, // Ban a client from the server. Must be host
    cf_KICK_CLIENT_FROM_SERVER     = 423, // Remove client from the server. Must be host

    // Message command
    cf_RELAY_MESSAGE_IN_SERVER  = 1840, // Send message from client to all clients in server 
    cf_RECV_CLIENT_SENT_MESSAGE = 1323, // Receive a message from client in a server and print it out
    // cf_SEND_MESSAGE_TO_ALL_CLIENTS = 1923, // Send client sent message to everyone in the current server
} cflags;


/**
 * Flags to tell the client what happened
 * After the response. Similar to rcodes
 */
typedef enum responseFlags
{
    rf_NONE            = 1,   // No flags
    rf_DATA_UPDATED    = 903, // The data was updated somehow.
    rf_NO_DATA_CHANGED = 923, // No data in the request was modified
    rf_NO_DATA_ADDED   = 103, // No new data was added. Return value is likely null
    rf_VALUE_RETURNED  = 823, // A value has been returned
    rf_NO_VALUE_RETURNED = -823, // No return value. Return value is null
} rflags;

/**
 * Response codes from the root server
 * After a request is made to it
 * Indicate what happened- is it successful or fail
 */
typedef enum responseCodes
{
    rc_SUCCESSFUL_OPERATION  = 0,    // Normal Success Code.
    rc_INTERNAL_SERVER_ERROR = -192, // Something went wrong on the servers side


} rcodes;
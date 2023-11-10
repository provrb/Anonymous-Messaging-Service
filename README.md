# The Anonymous Messaging Service or AMS for Short
WIP Command-line interface messaging application. Create temporary chat rooms and talk completely anonymously.

# AMS:

1. Never share passwords. Even though chats are end-to-end encrypted, it is best practice not to share sensitive information.

2. Chat completely anonymously. No signing up and all your chat logs are deleted alongside the server.

3. Create your own servers: Talk on your own servers with many other people or make them private.

_Better than IRC. More anonymous.
Currently only for Linux operating systems.
Created on Linux Mint in C/C++ with Visual Studio Code and compiled with GCC._

# REWORKING P2P PROCCESSES
Reworking the application so that it is more efficient.

The server will handle everything. A fully authentic coded server and requests like HTTP.
1. Migrating many client-sided processes to be handled server-sided
2. Better for an end-to-end encrypted.

# HOW CHATS ARE ENCRYPTED
The messages that pass to the server before reaching other clients in the chatroom are encrypted by AES-256 GCM.

An unbreakable bt brute-force encryption method used by the NSA and the US military.

## v1.0.1
- Major code cleanup
  - Organized code into respective header files
- Optimization

## v1.0.2
- Remove data flags. Old way to tell what information was being sent to the root server

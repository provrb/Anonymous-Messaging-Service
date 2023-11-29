# The Anonymous Messaging Service or AMS for Short
WIP Command-line interface messaging application. Create temporary chat rooms and talk completely anonymously.

# AMS:

1. Never share passwords. Even though chats are end-to-end encrypted, it is best practice not to share sensitive information.

2. Chat completely anonymously. No signing up and all your chat logs are deleted alongside the server.

3. Create your own servers: Talk on your own servers with many other people or make them private.

_Better than IRC. More anonymous.
Currently only for Linux operating systems.
Created on Linux Mint in C/C++ with Visual Studio Code and compiled with GCC._

# HOW CHATS ARE ENCRYPTED
Currently all chats are encrypted on the server by being xor'd.
Decrypted once a peer client receives the message. Simple encryption method.

~~The messages that pass to the server before reaching other clients in the CSServerChatroom are encrypted by AES-256 GCM.~~
~~An unbreakable brute-force encryption method used by the NSA and the US military.~~

## v1.0.1
- Major code cleanup
  - Organized code into respective header files
- Optimization

## v1.0.2
- Remove data flags. Old way to tell what information was being sent to the root server

## v1.1
- More documentation in header files
- Working Servers (May be bugs)
- Code cleanup
- Bug fixes
- Optimization
- Make requests to a server

# v1.1.5
- Major optimization
- Major code cleanup
- Bug fixes
  - Fixed segmentation fault on root server when client would disconnect
- Removed unneccessary functions
- Documentation in all header files now

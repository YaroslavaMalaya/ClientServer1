# General information
This code represents a simple client-server application over TCP where the server can transfer files to the client.
The functionality provides 5 commands to facilitate communication between the client and the server.

Protocol Type: TCP (Transmission Control Protocol)\
Port: 12345\
IP Address: localhost (127.0.0.1)\
Commands: Text-based commands with specific keywords (GET, LIST, PUT, DELETE, INFO, EXIT)\
Data Transfer Method: Binary\
Server Files Directory: /Users/Yarrochka/Mine/Study/KCT/lesson1/files/\
Client Files Directory: /Users/Yarrochka/Mine/Study/KCT/lesson1/clientfiles/

## Supported Commands
- GET <filename>\
Client requests a file from the server.\
Server copies the file from its directory to the client's directory.
- LIST\
Client requests a list of files in the server's directory.\
Server sends the list of files to the client.
- PUT <filename>\
Client sends a file to the server.\
Server saves the file in its directory.
- DELETE <filename>\
Client requests deletion of a file on the server.\
Server deletes the file if it exists.
- INFO <filename>\
Client requests information about a file on the server.\
Server sends file size, creation, and modification times.
- EXIT\
Client sends a command to terminate the connection.\
The client and server close their respective sockets.

## Server Side
Implementation\
Socket Creation: Uses socket(AF_INET, SOCK_STREAM, 0) for TCP socket.\
Binding: Binds the socket to the specified port with bind().\
Listening: Listens for connections using listen().\
Accepting Connections: Accepts client connections with accept().\
Command Processing: Parses commands received from the client and calls the appropriate function (getCommand, listCommand, etc.).\
File System Operations: Handles file operations using std::__fs::filesystem.\
Response Sending: Sends responses back to the client using send().

## Client side
Implementation\
Socket Creation: Uses socket(AF_INET, SOCK_STREAM, 0) for TCP socket.\
Connection: Connects to the server using connect().\
Command Input: Takes user input for commands and sends them to the server using send().\
Receiving Responses: Receives and displays server responses using recv().\
Looping for Commands: Continuously allows the user to input commands until EXIT is entered.\

## Server and Client Interaction
Initialization: The server must be running before the client attempts to connect.\
Command Order: The client always initiates the communication by sending a command. The server listens and responds accordingly.\
Data Transfer: For commands involving file transfers (GET, PUT), the server performs file system operations and sends a confirmation message.\
Response Waiting: After sending a command, the client waits for the server's response before proceeding to the next command.\
Clients Model: The server handles one client at a time. Multi-client handling is not implemented in this setup.

##  File System Access
Uses std::__fs::filesystem for file operations (C++17 feature).
In that way the program can transfer not only txt files, but also others type of files.

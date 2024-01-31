# General information
This code is written in C++. It represents a simple client-server application over TCP, where the server can handle 
multiple client connections concurrently. The functionality provides 5 commands to facilitate communication between the client and the server.

Protocol Type: TCP (Transmission Control Protocol)\
Port: 12345\
IP Address: localhost (127.0.0.1)\
Commands: Text-based commands with specific keywords (GET, LIST, PUT, DELETE, INFO, EXIT)\
Data Transfer Method: Binary\

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
The server uses threads to handle multiple client connections at the same time. 
Each client connection is managed by a separate thread.
Socket Creation: Uses socket(AF_INET, SOCK_STREAM, 0) for TCP socket.\
Binding: Binds the socket to the specified port with bind().\
Listening: Listens for connections using listen().\
Accepting Connections: Accepts client connections with accept() (in the while for each client).\
Command Processing: Parses commands received from the client and calls the appropriate function (getCommand, listCommand, etc.).\
File System Operations: Handles file operations using std::__fs::filesystem.\
Response Sending: Sends responses back to the client using send(). Here clientSocket should be provided, so the server knows in which client send the message.\
Mutexes: Uses to ensure that when one thread is using a shared resource (<cout> in the application), other threads
are prevented from accessing the same resource at the same time. This avoids data corruption.

## Client Side
Implementation\
At first, client should enter their name. This name is then used by the server to determine the corresponding client directory for file operations. \
Socket Creation: Uses socket(AF_INET, SOCK_STREAM, 0) for TCP socket.\
Connection: Connects to the server using connect().\
Command Input: Takes user input for commands and sends them to the server using send().\
Receiving Responses: Receives and displays server responses using recv().\
Looping for Commands: Continuously allows the user to input commands until EXIT is entered.\

## Server and Client Interaction
Initialization: The server must be running before the client attempts to connect.\
Thread: When the client connects, the server generates a new thread to handle client requests. This allows the server to manage multiple clients at the same time.\
Command Order: The client always initiates the communication by sending a command. The server listens and responds accordingly.\
Data Transfer: For commands involving file transfers (GET, PUT), the server performs file system operations and sends a confirmation message.\
Response Waiting: After sending a command, the client waits for the server's response before proceeding to the next command.\
Clients Model: The server handles one client at a time. Multi-client handling is not implemented in this setup.

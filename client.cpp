#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
using namespace std;

int main() {
    // Client configuration
    int port = 12345;
    const char* serverIp = "127.0.0.1";

    // Create a TCP socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating socket");
        return 1;
    }

    // Connect the socket to the server's address and port
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIp, &(serverAddr.sin_addr));

    if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        perror("Connect failed");
        close(clientSocket);
        return 1;
    }
    cout << "Available commands:\n- GET <filename>\n- LIST\n- PUT <filename>\n- DELETE <filename>\n- INFO <filename>\n- EXIT" << endl;

    while (true) {
        string command;
        cout << "Enter a command: ";
        getline(cin, command);

        if (command == "EXIT") {
            break;
        }

        // Send command to the server
        send(clientSocket, command.c_str(), command.size(), 0);

        if (command.find("GET ") == 0) {
            char fileMessage[4096];
            memset(fileMessage, 0, sizeof(fileMessage));
            ssize_t bytes = recv(clientSocket, fileMessage, sizeof(fileMessage), 0);
            if (bytes> 0) {
                cout << fileMessage << endl;
            }
        } else if (command == "LIST") {
            char listMessage[4096];
            memset(listMessage, 0, sizeof(listMessage));
            ssize_t bytes = recv(clientSocket, listMessage, sizeof(listMessage), 0);
            if (bytes > 0) {
                cout << "A list of files in the server directory:" << listMessage << endl;
            }
        } else if (command.find("PUT ") == 0) {
            char fileMessage[4096];
            memset(fileMessage, 0, sizeof(fileMessage));
            ssize_t bytes = recv(clientSocket, fileMessage, sizeof(fileMessage), 0);
            if (bytes> 0) {
                cout << fileMessage << endl;
            }
        } else if (command.find("DELETE ") == 0) {
            char fileMessage[4096];
            memset(fileMessage, 0, sizeof(fileMessage));
            ssize_t bytes = recv(clientSocket, fileMessage, sizeof(fileMessage), 0);
            if (bytes> 0) {
                cout << fileMessage << endl;
            }
        }
    }



    // Clean up
    close(clientSocket);
    return 0;
}

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <dirent.h>


int main() {
    // Server configuration
    int port = 12345;

    // Create a TCP socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        return 1;
    }

    // Bind the socket to a specific address and port
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        perror("Bind failed");
        close(serverSocket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == -1) {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    std::cout << "Server listening on port " << port << std::endl;

    // Accept a client connection
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
    if (clientSocket == -1) {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    std::cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;

    while (true) {
        char commandBuffer[1024];
        memset(commandBuffer, 0, sizeof(commandBuffer));
        ssize_t receivedCommand = recv(clientSocket, commandBuffer, sizeof(commandBuffer), 0);

        if (receivedCommand > 0) {
            std::string command(commandBuffer);

            if (command.find("GET ") == 0) {
                std::string filename = command.substr(4);
                std::string filePath = "/Users/Yarrochka/Mine/Study/KCT/lesson1/files/" + filename;
                std::ifstream file(filePath, std::ios::binary);

                if (file.is_open()) {
                    const char *message = "File was opened successfully.";
                    send(clientSocket, message, strlen(message), 0);
                } else {
                    std::cout << "Failed to open file '" << filePath << std::endl;
                    const char *message = "File not found or cannot be opened.";
                    send(clientSocket, message, strlen(message), 0);
                }
            } else if (command == "LIST") {
                std::string fileList;
                std::string directoryPath = "/Users/Yarrochka/Mine/Study/KCT/lesson1/files";

                for (const auto& files : std::__fs::filesystem::directory_iterator(directoryPath)) {
                    if (!files.is_directory()){
                        fileList += files.path().filename().string();
                        fileList += "\n";
                    }
                }
                send(clientSocket, fileList.c_str(), fileList.size(), 0);
            } else if (command.find("PUT ") == 0) {
                std::string filename = command.substr(4);
                std::string filePath = "/Users/Yarrochka/Mine/Study/KCT/lesson1/files/" + filename;
                std::ofstream newFile(filePath, std::ios::binary);

                if (newFile.is_open()) {
                    const char *confirmMessage = "File uploaded successfully.";
                    send(clientSocket, confirmMessage, strlen(confirmMessage), 0);
                } else {
                    const char *errorMessage = "Error creating file on server.";
                    send(clientSocket, errorMessage, strlen(errorMessage), 0);
                }
            }
        } else {
            perror("Received failed");
            break;
        }
    }

    // Clean up
    close(clientSocket);
    close(serverSocket);

    return 0;
}

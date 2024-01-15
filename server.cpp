#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <sstream>

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
                    const char *confirm = "File was opened successfully.";
                    send(clientSocket, confirm, strlen(confirm), 0);
                } else {
                    std::cout << "Failed to open file '" << filePath << std::endl;
                    const char *error = "File not found or cannot be opened.";
                    send(clientSocket, error, strlen(error), 0);
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
                std::string filePathClient = "/Users/Yarrochka/Mine/Study/KCT/lesson1/clientfiles/" + filename;
                std::string filePathServer = "/Users/Yarrochka/Mine/Study/KCT/lesson1/files/" + filename;
                std::ofstream newFile(filePathServer, std::ios::binary);
                std::__fs::filesystem::copy_file(filePathClient, filePathServer, std::__fs::filesystem::copy_options::overwrite_existing);

                if (newFile.is_open()) {
                    const char *confirm = "File uploaded successfully.";
                    send(clientSocket, confirm, strlen(confirm), 0);
                } else {
                    const char *error = "Error uploaded file on server.";
                    send(clientSocket, error, strlen(error), 0);
                }
            } else if (command.find("DELETE ") == 0) {
                std::string filename = command.substr(7);
                std::string filePath = "/Users/Yarrochka/Mine/Study/KCT/lesson1/files/" + filename;

                if (std::__fs::filesystem::remove(filePath)) {
                    const char *confirm = "File was deleted successfully.";
                    send(clientSocket, confirm, strlen(confirm), 0);
                } else {
                    const char *error = "The file cannot be deleted.";
                    send(clientSocket, error, strlen(error), 0);
                }
            } else if (command.find("INFO ") == 0) {
                std::string filename = command.substr(5);
                std::string filePath = "/Users/Yarrochka/Mine/Study/KCT/lesson1/files/" + filename;
                struct stat fileInfo;

                if (stat(filePath.c_str(), &fileInfo) != 0) {
                    std::cerr << "Error: " << strerror(errno) << std::endl;
                    const char *error = "Info cannot be obtained.";
                    send(clientSocket, error, strlen(error), 0);
                } else {
                    std::ostringstream information;
                    information << "Info.\n"
                               << "Size: " << fileInfo.st_size << "\n"
                               << "Created: " << std::asctime(std::localtime(&fileInfo.st_ctime))
                               << "Modified: " << std::asctime(std::localtime(&fileInfo.st_mtime));
                    std::string info = information.str();

                    send(clientSocket, info.c_str(), info.size(), 0);
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

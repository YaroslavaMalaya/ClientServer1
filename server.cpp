#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <sys/stat.h>
#include <sstream>

class Server{
private:
    int port = 12345;
    int serverSocket;
    int clientSocket;
    struct sockaddr_in serverAddr;
    sockaddr_in clientAddr;

    void listenSocket(){
        if (listen(serverSocket, SOMAXCONN) == -1) {
            perror("Listen failed");
            close(serverSocket);
        } else {
            std::cout << "Server listening on port " << port << std::endl;

            // Accept a client connection
            socklen_t clientAddrLen = sizeof(clientAddr);

            clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
            if (clientSocket == -1) {
                perror("Accept failed");
                close(serverSocket);
            } else {
                std::cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
            }
        }
    }

    void getCommand(std::string &command) {
        std::string filename = command.substr(4);
        std::string filePathServer = "/Users/Yarrochka/Mine/Study/KCT/lesson1/files/" + filename;
        std::string filePathClient = "/Users/Yarrochka/Mine/Study/KCT/lesson1/clientfiles/" + filename;
        std::ifstream file(filePathServer, std::ios::binary | std::ios::ate); // pointer in the end of the file since we use ate

        if (file.is_open()) {
            std::ofstream outFile(filePathClient, std::ios::binary);

            if (!outFile.is_open()) {
                std::cerr << "Failed to open file '" << filePathClient << "' for writing." << std::endl;
                const char *error = "File cannot be created.";
                send(clientSocket, error, strlen(error), 0);
                return;
            }

            std::streamsize fileSize = file.tellg();
            file.seekg(0, std::ios::beg); // move pointer on beginning

            // read and send the file in chunks
            while (fileSize > 0) {
                char buffer[1024];

                file.read(buffer, sizeof(buffer)); // read 1024 bytes
                std::streamsize bytes = file.gcount(); // the number of bytes that were last read from the file
                fileSize -= bytes;
                outFile.write(buffer, bytes);
            }

            const char *confirm = "File was opened and saved successfully.";
            send(clientSocket, confirm, strlen(confirm), 0);
        } else {
            std::cerr << "Failed to open file '" << filePathServer << "'" << std::endl;
            const char *error = "File not found or cannot be opened.";
            send(clientSocket, error, strlen(error), 0);
        }
    }


    void listCommand(){
        std::string fileList = "A list of files in the server directory:\n";
        std::string directoryPath = "/Users/Yarrochka/Mine/Study/KCT/lesson1/files";

        for (const auto& files : std::__fs::filesystem::directory_iterator(directoryPath)) {
            if (!files.is_directory()){
                fileList += files.path().filename().string();
                fileList += "\n";
            }
        }
        send(clientSocket, fileList.c_str(), fileList.size(), 0);
    }

    void putCommand(std::string &command){
        std::string filename = command.substr(4);
        std::string filePathServer = "/Users/Yarrochka/Mine/Study/KCT/lesson1/files/" + filename;
        std::string filePathClient = "/Users/Yarrochka/Mine/Study/KCT/lesson1/clientfiles/" + filename;
        std::ifstream file( filePathClient, std::ios::binary | std::ios::ate); // pointer in the end of the file since we use ate

        if (file.is_open()) {
            std::ofstream outFile(filePathServer, std::ios::binary);

            if (!outFile.is_open()) {
                std::cerr << "Failed to open file '" << filePathServer << "' for writing." << std::endl;
                const char *error = "File cannot be created.";
                send(clientSocket, error, strlen(error), 0);
                return;
            }

            std::streamsize fileSize = file.tellg();
            file.seekg(0, std::ios::beg); // move pointer on beginning

            // read and send the file in chunks
            while (fileSize > 0) {
                char buffer[1024];

                file.read(buffer, sizeof(buffer)); // read 1024 bytes
                std::streamsize bytes = file.gcount(); // the number of bytes that were last read from the file
                fileSize -= bytes;
                outFile.write(buffer, bytes);
            }

            const char *confirm = "File was opened and saved successfully.";
            send(clientSocket, confirm, strlen(confirm), 0);
        } else {
            std::cerr << "Failed to open file '" << filePathServer << "'" << std::endl;
            const char *error = "File not found or cannot be opened.";
            send(clientSocket, error, strlen(error), 0);
        }
    }

    void deleteCommand(std::string &command){
        std::string filename = command.substr(7);
        std::string filePath = "/Users/Yarrochka/Mine/Study/KCT/lesson1/files/" + filename;

        if (std::__fs::filesystem::remove(filePath)) {
            const char *confirm = "File was deleted successfully.";
            send(clientSocket, confirm, strlen(confirm), 0);
        } else {
            const char *error = "The file cannot be deleted.";
            send(clientSocket, error, strlen(error), 0);
        }
    }

    void infoCommand(std::string &command){
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

public:
    Server(){
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            perror("Error creating socket");
        } else {
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = INADDR_ANY;
            serverAddr.sin_port = htons(port);

            if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
                perror("Bind failed");
                close(serverSocket);
            } else {
                listenSocket();
            }
        }
    }

    ~Server() {
        close(clientSocket);
        close(serverSocket);
    }

    void handlingCommands(){
        while (true) {
            char commandBuffer[1024];
            memset(commandBuffer, 0, sizeof(commandBuffer));
            ssize_t receivedCommand = recv(clientSocket, commandBuffer, sizeof(commandBuffer), 0);

            if (receivedCommand > 0) {
                std::string command(commandBuffer);

                if (command.find("GET ") == 0) {
                    getCommand(command);
                } else if (command == "LIST") {
                    listCommand();
                } else if (command.find("PUT ") == 0) {
                    putCommand(command);
                } else if (command.find("DELETE ") == 0) {
                    deleteCommand(command);
                } else if (command.find("INFO ") == 0) {
                    infoCommand(command);
                }
            } else {
                perror("Received failed");
                break;
            }
        }
    }
};

int main() {
    Server newServer;
    newServer.handlingCommands();
    return 0;
}

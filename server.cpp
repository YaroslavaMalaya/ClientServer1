#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <sys/stat.h>
#include <sstream>
#include <thread>
#include <mutex>

class Server{
private:
    int port = 12345;
    int serverSocket;
    std::mutex m;
    struct sockaddr_in serverAddr;
    sockaddr_in clientAddr;
    std::vector<std::thread> clientThreads;

    void listenSocket(){
        if (listen(serverSocket, SOMAXCONN) == -1) {
            perror("Listen failed");
            close(serverSocket);
        } else {
            m.lock();
            std::cout << "Server listening on port " << port << std::endl;
            m.unlock();

            // Accept a client connection
            while (true) {
                socklen_t clientAddrLen = sizeof(clientAddr);

                int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
                if (clientSocket == -1) {
                    perror("Error accepting client connection");
                    break;
                }

                m.lock();
                std::cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
                m.unlock();

                // Create a new thread for each client connection
                clientThreads.emplace_back(&Server::handlingCommands, this, clientSocket);
            }
        }
    }

    void getCommand(std::string &command, int clientSocket, const std::string& path1, const std::string& path2) {
        std::string filename = command.substr(4);
        std::string filePathServer = path1 + filename;
        std::string filePathClient = path2 + filename;
        std::ifstream file(filePathServer, std::ios::binary | std::ios::ate); // pointer in the end of the file since we use ate

        if (file.is_open()) {
            std::ofstream outFile(filePathClient, std::ios::binary);

            if (!outFile.is_open()) {
                std::cerr << "Failed to open file '" << filePathClient << "' for writing." << std::endl;
                const char *error = "File cannot be created.";
                m.lock();
                send(clientSocket, error, strlen(error), 0);
                m.unlock();
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
            m.lock();
            send(clientSocket, confirm, strlen(confirm), 0);
            m.unlock();
        } else {
            std::cerr << "Failed to open file '" << filePathServer << "'" << std::endl;
            const char *error = "File not found or cannot be opened.";
            m.lock();
            send(clientSocket, error, strlen(error), 0);
            m.unlock();
        }
    }

    void listCommand(int clientSocket, const std::string& path1){
        std::string fileList = "A list of filesClient1 in the server directory:\n";
        std::string directoryPath = path1;

        for (const auto& files : std::__fs::filesystem::directory_iterator(directoryPath)) {
            if (!files.is_directory()){
                fileList += files.path().filename().string();
                fileList += "\n";
            }
        }
        m.lock();
        send(clientSocket, fileList.c_str(), fileList.size(), 0);
        m.unlock();
    }

    void putCommand(std::string &command, int clientSocket, const std::string& path1, const std::string& path2){
        std::string filename = command.substr(4);
        std::string filePathServer = path1 + filename;
        std::string filePathClient = path2 + filename;
        std::ifstream file( filePathClient, std::ios::binary | std::ios::ate); // pointer in the end of the file since we use ate

        if (file.is_open()) {
            std::ofstream outFile(filePathServer, std::ios::binary);

            if (!outFile.is_open()) {
                std::cerr << "Failed to open file '" << filePathServer << "' for writing." << std::endl;
                const char *error = "File cannot be created.";
                m.lock();
                send(clientSocket, error, strlen(error), 0);
                m.unlock();
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
            m.lock();
            send(clientSocket, confirm, strlen(confirm), 0);
            m.unlock();
        } else {
            m.lock();
            std::cerr << "Failed to open file '" << filePathServer << "'" << std::endl;
            const char *error = "File not found or cannot be opened.";
            send(clientSocket, error, strlen(error), 0);
            m.unlock();
        }
    }

    void deleteCommand(std::string &command, int clientSocket, const std::string& path1){
        std::string filename = command.substr(7);
        std::string filePath = path1 + filename;

        if (std::__fs::filesystem::remove(filePath)) {
            const char *confirm = "File was deleted successfully.";
            m.lock();
            send(clientSocket, confirm, strlen(confirm), 0);
            m.unlock();
        } else {
            const char *error = "The file cannot be deleted.";
            m.lock();
            send(clientSocket, error, strlen(error), 0);
            m.unlock();
        }
    }

    void infoCommand(std::string &command, int clientSocket, const std::string& path1){
        std::string filename = command.substr(5);
        std::string filePath = path1 + filename;
        struct stat fileInfo;

        if (stat(filePath.c_str(), &fileInfo) != 0) {
            std::cerr << "Error: " << strerror(errno) << std::endl;
            const char *error = "Info cannot be obtained.";
            m.lock();
            send(clientSocket, error, strlen(error), 0);
            m.unlock();
        } else {
            std::ostringstream information;
            information << "Info.\n"
                        << "Size: " << fileInfo.st_size << "\n"
                        << "Created: " << std::asctime(std::localtime(&fileInfo.st_ctime))
                        << "Modified: " << std::asctime(std::localtime(&fileInfo.st_mtime));
            std::string info = information.str();
            m.lock();
            send(clientSocket, info.c_str(), info.size(), 0);
            m.unlock();
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
                return;
            } else {
                listenSocket();
            }
        }
    }

    ~Server() {
        close(serverSocket);

        for (auto& thread : clientThreads) {
            thread.join();
        }
    }

    void setPaths(int clientSocket, std::string& clientFolderPath, std::string& serverFolderPath) {
        char clientNameBuffer[1024];
        memset(clientNameBuffer, 0, sizeof(clientNameBuffer));
        ssize_t receivedName = recv(clientSocket, clientNameBuffer, sizeof(clientNameBuffer), 0);
        if (receivedName <= 0) {
            perror("Failed to receive client name");
            close(clientSocket);
            return;
        }

        std::string clientName(clientNameBuffer);

        if (clientName == "client1") {
            clientFolderPath = "/Users/Yarrochka/Mine/Study/CSC/lesson1/client1/";
            serverFolderPath = "/Users/Yarrochka/Mine/Study/CSC/lesson1/filesClient1/";
        } else if (clientName == "client2") {
            clientFolderPath = "/Users/Yarrochka/Mine/Study/CSC/lesson1/client2/";
            serverFolderPath = "/Users/Yarrochka/Mine/Study/CSC/lesson1/filesClient2/";
        }
    }

    void handlingCommands(int clientSocket){
        std::string clientFolderPath;
        std::string serverFolderPath;

        setPaths(clientSocket, clientFolderPath, serverFolderPath);

        while (true) {
            char commandBuffer[1024];
            memset(commandBuffer, 0, sizeof(commandBuffer));
            ssize_t receivedCommand = recv(clientSocket, commandBuffer, sizeof(commandBuffer), 0);

            if (receivedCommand > 0) {
                std::string command(commandBuffer);

                if (command.find("GET ") == 0) {
                    getCommand(command, clientSocket, serverFolderPath, clientFolderPath);
                } else if (command == "LIST") {
                    listCommand(clientSocket, serverFolderPath);
                } else if (command.find("PUT ") == 0) {
                    putCommand(command, clientSocket, serverFolderPath, clientFolderPath);
                } else if (command.find("DELETE ") == 0) {
                    deleteCommand(command, clientSocket, serverFolderPath);
                } else if (command.find("INFO ") == 0) {
                    infoCommand(command, clientSocket, serverFolderPath);
                }
            } else {
                perror("Received failed");
                break;
            }
        }
        close(clientSocket);
    }
};

int main() {
    Server newServer;
    return 0;
}

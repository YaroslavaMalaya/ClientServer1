#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <sys/stat.h>
#include <sstream>
#include <thread>
#include <mutex>
#include "file.h"

std::mutex m;
class MacServerConnection{
private:
    int serverSocket;
    struct sockaddr_in serverAddr;

public:
    MacServerConnection(int port) {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            error("Error creating socket");
        } else {
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = INADDR_ANY;
            serverAddr.sin_port = htons(port);

            if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
                error("Bind failed");
                close(serverSocket);
            }
        }
    }

    void error(const char* message) const {
        m.lock();
        perror(message);
        m.unlock();
    }

    int listenConnection() const {
        if (listen(serverSocket, SOMAXCONN) == -1) {
            error("Listen failed");
            return -1;
        }
        return 0;
    }

    int acceptConnection(struct sockaddr_in &clientAddr) const {
        socklen_t clientAddrLen = sizeof(clientAddr);
        return accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
    }

    void sendM(int clientSocket, const char* buffer, size_t length, int num) const {
        send(clientSocket, buffer, length, num);
    }

    ssize_t recvM(int clientSocket, char* buffer, size_t length, int num) const {
        ssize_t receivedBytes = recv(clientSocket, buffer, length, num);
        return receivedBytes;
    }

    void closeConnection(){
        close(serverSocket);
    }
};


class Server{
private:
    int port = 12345;
    sockaddr_in clientAddr;
    MacServerConnection macServer;
    std::vector<std::thread> clientThreads;

    void listenSocket(){
        if (macServer.listenConnection() == -1) {
            return;
        } else {
            std::cout << "Server listening on port " << port << std::endl;

            // Accept a client connection
            while (true) {

                int clientSocket = macServer.acceptConnection(clientAddr);
                if (clientSocket == -1) {
                    macServer.error("Error accepting client connection");
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

        m.lock();
        std::cout << "GET request for '" << filename << "' from client " << clientSocket << std::endl;
        m.unlock();

        std::string com = command.substr(3);
        File::copyFile(filePathServer, filePathClient, clientSocket, com);
    }

    void listCommand(int clientSocket, const std::string& path1){
        std::string fileList = "A list of filesClient1 in the server directory:\n";
        const std::string& directoryPath = path1;

        m.lock();
        std::cout << "LIST request from client " << clientSocket << std::endl;
        m.unlock();

        for (const auto& files : std::__fs::filesystem::directory_iterator(directoryPath)) {
            if (!files.is_directory()){
                fileList += files.path().filename().string();
                fileList += "\n";
            }
        }
        macServer.sendM(clientSocket, fileList.c_str(), fileList.size(), 0);

        m.lock();
        std::cout << "LIST response sent to client " << clientSocket << std::endl;
        m.unlock();
    }

    void putCommand(std::string &command, int clientSocket, const std::string& path1, const std::string& path2){
        std::string filename = command.substr(4);
        std::string filePathServer = path1 + filename;
        std::string filePathClient = path2 + filename;

        m.lock();
        std::cout << "PUT request for '" << filename << "' from client " << clientSocket << std::endl;
        m.unlock();

        std::string com = command.substr(3);
        File::copyFile(filePathClient, filePathServer, clientSocket, com);
    }

    void deleteCommand(std::string &command, int clientSocket, const std::string& path1){
        std::string filename = command.substr(7);
        std::string filePath = path1 + filename;

        m.lock();
        std::cout << "DELETE request for '" << filename << "' from client " << clientSocket << std::endl;
        m.unlock();

        if (std::__fs::filesystem::remove(filePath)) {
            const char *confirm = "File was deleted successfully.";
            macServer.sendM(clientSocket, confirm, strlen(confirm), 0);

            m.lock();
            std::cout << "DELETE response sent to client " << clientSocket << std::endl;
            m.unlock();
        } else {
            const char *error = "The file cannot be deleted.";
            macServer.sendM(clientSocket, error, strlen(error), 0);
        }
    }

    void infoCommand(std::string &command, int clientSocket, const std::string& path1){
        std::string filename = command.substr(5);
        std::string filePath = path1 + filename;

        m.lock();
        std::cout << "INFO request for '" << filename << "' from client " << clientSocket << std::endl;
        m.unlock();

        File::getInfo(filePath,clientSocket);
    }

    void infoCommandClient(std::string &command, int clientSocket, const std::string& path1){
        std::string filename = command.substr(6);
        std::string filePath = path1 + filename;

        m.lock();
        std::cout << "INFOC request for '" << filename << "' from client " << clientSocket << std::endl;
        m.unlock();

        File::getInfo(filePath,clientSocket);
    }

public:
    Server() : macServer(port) {
        listenSocket();
    }

    ~Server() {
        macServer.closeConnection();

        for (auto& thread : clientThreads) {
            thread.join();
        }
    }

    void setPaths(int clientSocket, std::string& clientFolderPath, std::string& serverFolderPath) {
        char clientNameBuffer[1024];
        memset(clientNameBuffer, 0, sizeof(clientNameBuffer));
        ssize_t receivedName = macServer.recvM(clientSocket, clientNameBuffer, sizeof(clientNameBuffer), 0);
        if (receivedName <= 0) {
            macServer.error("Failed to receive client name");
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
            ssize_t receivedCommand = macServer.recvM(clientSocket, commandBuffer, sizeof(commandBuffer), 0);

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
                } else if (command.find("INFOC ") == 0) {
                    infoCommandClient(command, clientSocket, clientFolderPath);
                }
            } else {
                macServer.error("Received failed");
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

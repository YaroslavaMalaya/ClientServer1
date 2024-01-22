#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
using namespace std;

class Client{
private:
    int port = 12345;
    const char* serverIp = "127.0.0.1";
    int clientSocket;
    struct sockaddr_in serverAddr;

public:
    Client(){
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            perror("Error creating socket");
        }
        else{
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(port);
            inet_pton(AF_INET, serverIp, &(serverAddr.sin_addr));

            if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
                perror("Connect failed");
                close(clientSocket);
            }
        }
    }

    void receiveServerMessage() const {
        char fileMessage[1024];
        memset(fileMessage, 0, sizeof(fileMessage));
        ssize_t bytes = recv(clientSocket, fileMessage, sizeof(fileMessage), 0);
        if (bytes > 0) {
            cout << fileMessage << endl;
        } else {
            cerr << "Failed to receive message from server." << endl;
        }
    }

    void commands() const{
        cout << "Available commands:\n- GET <filename>\n- LIST\n- PUT <filename>\n- DELETE <filename>\n- INFO <filename>\n- EXIT" << endl;

        while (true) {
            string command;
            cout << "Enter a command: ";
            getline(cin, command);

            if (command == "EXIT") {
                break;
            }

            send(clientSocket, command.c_str(), command.size(), 0);

            if (command.find("GET ") == 0 || command == "LIST" || command.find("PUT ") == 0 ||
                command.find("DELETE ") == 0 || command.find("INFO ") == 0) {
                receiveServerMessage();
            } else {
                cout << "Unknown command. TRy again." << endl;
            }
        }
    }

    ~Client() {
        close(clientSocket);
    }

};

int main() {
    Client newClient;
    newClient.commands();
    return 0;
}

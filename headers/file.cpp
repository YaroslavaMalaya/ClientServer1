#include "file.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>

std::mutex m;

void File::copyFile(const std::string& path1, const std::string& path2, const int socket, const std::string& com) {
    std::ifstream file(path1, std::ios::binary | std::ios::ate); // pointer in the end of the file since we use ate

    if (file.is_open()) {
        std::ofstream outFile(path2, std::ios::binary);

        if (!outFile.is_open()) {
            m.lock();
            std::cerr << "Failed to open file '" << path2 << "' for writing." << std::endl;
            m.unlock();
            const char *error = "File cannot be created.";
            send(socket, error, strlen(error), 0);
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
        send(socket, confirm, strlen(confirm), 0);

        m.lock();
        std::cout << com << " response sent to client " << socket << std::endl;
        m.unlock();
    } else {
        m.lock();
        std::cerr << "Failed to open file '" << path1 << "'" << std::endl;
        m.unlock();
        const char *error = "File not found or cannot be opened.";
        send(socket, error, strlen(error), 0);
    }
}

void File::getInfo(const std::string& path1, const int socket) {
    struct stat info;

    if (stat(path1.c_str(), &info) != 0) {
        m.lock();
        std::cerr << "Error: " << strerror(errno) << std::endl;
        m.unlock();
        const char *error = "Info cannot be obtained.";
        send(socket, error, strlen(error), 0);
    } else {
        std::ostringstream information;
        information << "Info.\n"
                    << "Size: " << info.st_size << "\n"
                    << "Created: " << std::asctime(std::localtime(&info.st_ctime))
                    << "Modified: " << std::asctime(std::localtime(&info.st_mtime));
        std::string info = information.str();
        send(socket, info.c_str(), info.size(), 0);

        m.lock();
        std::cout << "INFO response sent to client " << socket << std::endl;
        m.unlock();
    }
}
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class Logger {
    private:
        std::ofstream outFile;

        void saveToFile(std::string event);

    public:
        Logger();
        Logger(fs::path filePath);
        // ~Logger();

        void serverStart();
        void newClientConnection(std::string ipAddress);
        void loginAttempt(std::string username, std::string ipAddress, bool wasSucessful);
        void clientDisconnected(std::string ipAddress);

        void matchStarted(std::string username1, std::string ipAddress1, std::string username2, std::string ipAddress2);

        void matchEnded(std::string username1, std::string ipAddress1, std::string username2, std::string ipAddress2, std::string winnerUsername);

        void unexpectedClientDisconnect(std::string ipAddress);
        void serverEnd();

        void close();
};

#endif
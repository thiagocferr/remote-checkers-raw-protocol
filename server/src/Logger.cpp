
#include "Logger.hpp"

#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>


static std::string getCurrentTime() {
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);

    std::string val(
        std::to_string(now->tm_year + 1900) + "-" +
        std::to_string(now->tm_mon + 1) + "-" +
        std::to_string(now->tm_mday) + " " +

        std::to_string(now->tm_hour + 1) + ":" +
        std::to_string(now->tm_min + 1) + ":" +
        std::to_string(now->tm_sec)
    );

    return val;
}

void Logger::saveToFile(std::string event) {
    this->outFile << "[" + getCurrentTime() + "] - " + event << std::endl;
}

Logger::Logger() {}

Logger::Logger(fs::path filePath) {
    this->outFile = std::ofstream(filePath, std::ios_base::app | std::ios_base::out);

}


void Logger::serverStart() {
    this->saveToFile("Server process has been started");
}

void Logger::newClientConnection(std::string ipAddress) {
    this->saveToFile("Client at \"" + ipAddress + "\" has connected");
}

void Logger::loginAttempt(std::string username, std::string ipAddress, bool wasSucessful) {
    if (wasSucessful)
        this->saveToFile("User \"" + username + "\" was successfully logged in from IP \"" + ipAddress + "\"");
    else
        this->saveToFile("Unsuccessful login attemp to user \"" + username + "\" from IP \"" + ipAddress + "\"");
}

void Logger::clientDisconnected(std::string ipAddress) {
    this->saveToFile("Client at IP \"" + ipAddress + "\" disconnected from server gracefully");
}

void Logger::matchStarted(std::string username1, std::string ipAddress1,
                          std::string username2, std::string ipAddress2) {
    this->saveToFile("A match has started between \"" + username1 + "\" (" + ipAddress1 + ") and \"" + username2 + "\" (" + ipAddress2 + ")");
}

void Logger::matchEnded(std::string username1, std::string ipAddress1,
                        std::string username2, std::string ipAddress2,
                        std::string winnerUsername) {
    return this->saveToFile("A match has ended between \"" + username1 + "\" (" + ipAddress1 + ") and \"" + username2 + "\" (" + ipAddress2 + ") - WINNER: " + winnerUsername);
}

void Logger::unexpectedClientDisconnect(std::string ipAddress) {
    return this->saveToFile("Client at IP \"" + ipAddress + "\" disconnected from server unexpectedly");
}

void Logger::serverEnd() {
    return this->saveToFile("Server process has been terminated");
}

void Logger::close() {
    this->outFile.close();
}
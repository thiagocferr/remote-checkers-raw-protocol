#ifndef PLAYER_HPP
#define PLAYER_HPP

// #include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class Player {
    private:
        std::string username;
        std::string password;

        std::string ipAddress;

        int score;
        bool isOnline;
        bool isOnMatch;

        // Since the player's loading into memory, mark if some information has been
        // changed (for persistence purposes)
        bool isDirty;

        // The write side of the pipe where other player can send match requests to.
        // -1 if current player cannot be reached (offline)
        int threadPipeWriter;

        std::string enemyUsername;

    public:
        // Load player from file
        Player(const fs::path& path);
        // Create new player
        Player(const std::string username, const std::string password);

        std::string getUsername() const;

        void setPassword(const std::string password);
        std::string getPassword();

        void setScore(int newScore);
        int getScore();

        void setIpAddress(std::string ipAddress);
        std::string getIpAdress();

        void activate(int threadPipeWriter);
        void deactivate();
        bool isOn();

        void startGame(const std::string enemyUsername);
        void endGame();
        bool isOnGame();

        std::string getEnemyUsername();

        bool hasChanged();
        int getPipeWriter();

        void saveToFile(fs::path path);
};

#endif

#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include "Player.hpp"

Player::Player(const fs::path& path) {

    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();

    // Get info from file
    std::vector<std::string> entries;
    for (std::string line; std::getline(buffer, line); ) {
        auto first = line.find(" ");

        if (first == std::string::npos)
            std::cerr << "Invalid player file!" << std::endl;

        entries.push_back(line.substr(first + 1, -1));
    }

    this->username = entries[0];
    this->password = entries[1];
    this->score = std::stoi(entries[2]);

    this->isOnline = false;
    this->isOnMatch = false;
    this->enemyUsername = "";

    this->isDirty = false;
    this->threadPipeWriter = -1;

}

Player::Player(const std::string username, const std::string password) {
    this->username = username;
    this->password = password;
    this->score = 0;
    this->isOnline = false;
    this->isOnMatch = false;
    this->enemyUsername = "";

    this->isDirty = true;
    this->threadPipeWriter = -1;
}

std::string Player::getUsername() const {
    return this->username;
}

void Player::setPassword(const std::string password) {
    if(password == this->password)
        this->isDirty = true;

    this->password = password;
}
std::string Player::getPassword() {
    return this->password;
}

void Player::setScore(int newScore) {
    if(newScore == this->score)
        this->isDirty = true;

    this->score = newScore;
    this->isDirty = true;
}
int Player::getScore() {
    return this->score;
}

void Player::setIpAddress(std::string ipAddress) {
    this->ipAddress = ipAddress;
}
std::string Player::getIpAdress() {
    return this->ipAddress;
}

void Player::activate(int threadPipeWriter) {
    this->isOnline = true;
    this->threadPipeWriter = threadPipeWriter;
}
void Player::deactivate() {
    this->isOnline = false;
    this->threadPipeWriter = -1;
    this->ipAddress = "";
}
bool Player::isOn() {
    return this->isOnline;
}

void Player::startGame(const std::string enemyUsername) {
    this->enemyUsername = enemyUsername;
    this->isOnMatch = true;
}
void Player::endGame() {
    this->enemyUsername = "";
    this->isOnMatch = false;
}

bool Player::isOnGame() {
    return this->isOnMatch;
}

std::string Player::getEnemyUsername() {
    return this->enemyUsername;
}



bool Player::hasChanged() {
    return this->isDirty;
}
int Player::getPipeWriter() {
    return this->threadPipeWriter;
}

void Player::saveToFile(fs::path path) {

    std::string str;
    path /= this->username;
    std::ofstream file(path);

    str += "USERNAME " + this->username + "\n";
    str += "PASSWORD " + this->password + "\n";
    str += "SCORE " + std::to_string(this->score);

    file <<  str;
    file.close();

    this->isDirty = false;
}

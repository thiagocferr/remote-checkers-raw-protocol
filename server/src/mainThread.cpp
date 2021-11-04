#include "mainThread.hpp"

#include <iostream>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <fstream>
#include <csignal>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>

#include "Player.hpp"
#include "Communication.hpp"
#include "Tokenizer.hpp"
#include "Logger.hpp"

namespace fs = std::filesystem;

// Set of macros for transforming a macro into string literal
#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

Logger logger;

void handleSeverFinish([[maybe_unused]]int s) {
    logger.serverEnd();
    logger.close();
    exit(0);
}

// Load all players into memory
void loadAllPlayers(std::string storedPlayersDir) {

    // Loading players into memory
    auto it = fs::directory_iterator(storedPlayersDir);
    auto totalNumOfPlayers = std::distance(it, {});
    allPlayers.reserve(totalNumOfPlayers);

    for (const auto & entry : fs::directory_iterator(storedPlayersDir)) {

        Player existingPlayer(entry.path());

        // This is done (method 'emplace'), instead of the usual creation via []-operator, because the non-presence
        // of an element with a certain key will trigger a default constructor from Player class, which
        // does not exist
        allPlayers.emplace(existingPlayer.getUsername(), existingPlayer);
    }

}

void mainThread(int mainPipeReader) {

    // Loading all existing players to memory. Searched directory path can vary, depending on
    // compilation flag used (this is all setup in makefiles)
    #ifdef SERVER_DIR
    std::string serverDir = STRINGIFY(SERVER_DIR);
    #else
    std::string serverDir = ".";
    #endif

    std::string storedPlayersDir = serverDir + "/players";
    loadAllPlayers(storedPlayersDir);

    std::string logFilePath = serverDir + "/logFile.log";
    logger = Logger(logFilePath);
    logger.serverStart();

    signal(SIGINT, handleSeverFinish);

    while (true) {

        std::pair<bool, std::vector<std::string>> messages = Communication::readPipe(mainPipeReader);

        for (std::string message : messages.second) {

            TokenMatrix inTokenMatrix(std::move(message));
            TokenMatrix outTokenMatrix;

            int lineIndex = -1;
            int threadPipeWriter = -1;
            lineIndex = inTokenMatrix.findLineIndex("THREAD_ID");

            // Get where to write response message. If no place is specified, ignore message and don't
            // send anything (not that it could without a reference to which pipe to send to)
            if (lineIndex != -1) {
                std::string threadID = inTokenMatrix[lineIndex][1];
                threadIdToPipeMutex.lock();
                    auto search = threadIdToPipe.find(threadID);
                    if (search != threadIdToPipe.end())
                        threadPipeWriter = search->second;
                    else
                        continue;
                threadIdToPipeMutex.unlock();
            }

            // New connection to client has been stablished
            if (inTokenMatrix[0][0] == "CONNIN") {
                logger.newClientConnection(inTokenMatrix[0][1]);
                continue;
            }

            // Treat 'adduser' user command
            else if (inTokenMatrix[0][0] == "ADDUSER") {
                std::string username = inTokenMatrix[0][1];
                std::string password = inTokenMatrix[0][2];

                outTokenMatrix.addText("ADDUSERACK " + username);

                if (allPlayers.find(username) == allPlayers.end()) {
                    Player newPlayer(username, password);
                    allPlayers.emplace(username, newPlayer);
                    newPlayer.saveToFile(storedPlayersDir);

                    outTokenMatrix.addText("STATUS ok");
                } else {

                    outTokenMatrix.addText("STATUS fail already_exist");
                }
            }

            // Treat 'login' user command
            else if (inTokenMatrix[0][0] == "LOGIN") {
                std::string username = inTokenMatrix[0][1];
                std::string password = inTokenMatrix[0][2];

                int i = inTokenMatrix.findLineIndex("IP");
                std::string ipAddress = inTokenMatrix[i][1];

                outTokenMatrix.addText("LOGINACK " + username);

                auto it = allPlayers.find(username);
                if (it != allPlayers.end()) {
                    Player& existingPlayer = it->second;
                    if (existingPlayer.getPassword() == password) {

                        if (existingPlayer.isOn()) {
                            outTokenMatrix.addText("STATUS fail already_logged");
                            logger.loginAttempt(username, ipAddress, false);
                        } else {


                            existingPlayer.activate(threadPipeWriter);
                            existingPlayer.setIpAddress(ipAddress);
                            logger.loginAttempt(username, ipAddress, true);

                            outTokenMatrix.addText("STATUS ok");
                        }
                    } else {
                        outTokenMatrix.addText("STATUS fail wrong_username_or_password");
                        logger.loginAttempt(username, ipAddress, false);
                    }
                } else {
                    outTokenMatrix.addText("STATUS fail wrong_username_or_password");
                    logger.loginAttempt(username, ipAddress, false);
                }
            }

            // Treat 'passwd' user command
            else if (inTokenMatrix[0][0] == "PASSWD") {
                std::string oldPassword = inTokenMatrix[0][1];
                std::string newPassword = inTokenMatrix[0][2];

                int i = inTokenMatrix.findLineIndex("USERNAME");
                std::string username = inTokenMatrix[i][1];

                outTokenMatrix.addText("PASSWDACK " + username);

                Player& player = allPlayers.find(username)->second;
                if (player.getPassword() == oldPassword) {
                    player.setPassword(newPassword);
                    outTokenMatrix.addText("STATUS ok");
                } else {
                    outTokenMatrix.addText("STATUS fail wrong_password");
                }
            }

            else if (inTokenMatrix[0][0] == "LEADERS") {

                std::string leaderboard = "";
                for (auto & [username, player] : allPlayers) {
                    leaderboard += username + " " + std::to_string(player.getScore()) + " ";
                }
                if (leaderboard.size() > 0)
                    leaderboard.pop_back();

                outTokenMatrix.addText("LEADERS " + leaderboard);
            }

            else if (inTokenMatrix[0][0] == "LIST") {
                std::string list = "";
                for (auto & [username, player] : allPlayers) {
                    if (player.isOn()) {
                        list += username + " " + (player.isOnGame() ? "YES" : "NO") + " ";
                    }
                }
                if (list.size() > 0)
                    list.pop_back();

                outTokenMatrix.addText("LIST " + list);
            }

            else if (inTokenMatrix[0][0] == "LOGOUT") {
                int i = inTokenMatrix.findLineIndex("USERNAME");
                std::string username = inTokenMatrix[i][1];

                Player& player = allPlayers.find(username)->second;
                player.deactivate();

                outTokenMatrix.addText("LOGOUTACK " + username);
            }

            else if (inTokenMatrix[0][0] == "CANCEL") {
                int i = inTokenMatrix.findLineIndex("USERNAME");
                std::string fromUsername = inTokenMatrix[i][1];

                std::string cancelInviteTo = inTokenMatrix[0][1];


                auto it = allPlayers.find(cancelInviteTo);
                if (it != allPlayers.end()) {
                    Player& player = it->second;
                    if (player.isOn()) {
                        int otherThreadPipeWriter = player.getPipeWriter();

                        TokenMatrix otherOutTokenMatrix;
                        otherOutTokenMatrix.addText("CANCEL " + cancelInviteTo);
                        otherOutTokenMatrix.addText("FROM " + fromUsername);
                        Communication::writeToPipe(otherThreadPipeWriter, otherOutTokenMatrix.str());
                        continue;
                    }
                }
            }

            else if (inTokenMatrix[0][0] == "BEGINREQ") {
                int i = inTokenMatrix.findLineIndex("FROM");

                std::string fromUsername = inTokenMatrix[i][1];
                std::string toUsername = inTokenMatrix[0][1];

                outTokenMatrix.addText("BEGINREQACK " + toUsername);
                outTokenMatrix.addText("FROM " + fromUsername);

                auto it = allPlayers.find(toUsername);
                if (it != allPlayers.end()) {
                    Player& player = it->second;
                    if (player.isOn()) {
                        int otherThreadPipeWriter = player.getPipeWriter();

                        TokenMatrix otherOutTokenMatrix;
                        otherOutTokenMatrix.addText("BEGINREQ " + toUsername);
                        otherOutTokenMatrix.addText("FROM " + fromUsername);
                        Communication::writeToPipe(otherThreadPipeWriter, otherOutTokenMatrix.str());
                        continue;

                    } else {
                        outTokenMatrix.addText("STATUS fail player_offline");
                    }
                } else {
                    outTokenMatrix.addText("STATUS fail player_not_exists");
                }
            }

            else if (inTokenMatrix[0][0] == "BEGINRESP") {
                int i = inTokenMatrix.findLineIndex("FROM");

                std::string toUsername = inTokenMatrix[0][1];
                std::string response = inTokenMatrix[0][2];
                std::string fromUsername = inTokenMatrix[i][1];

                outTokenMatrix.addText("BEGINRESPACK " + toUsername + " " + response);
                outTokenMatrix.addText("FROM " + fromUsername);

                auto it = allPlayers.find(toUsername);
                if (it != allPlayers.end()) {
                    Player& player = it->second;
                    if (player.isOn()) {
                        int otherThreadPipeWriter = player.getPipeWriter();

                        TokenMatrix otherOutTokenMatrix;
                        otherOutTokenMatrix.addText("BEGINRESP " + toUsername + " " + response);
                        otherOutTokenMatrix.addText("FROM " + fromUsername);
                        Communication::writeToPipe(otherThreadPipeWriter, otherOutTokenMatrix.str());
                        continue;

                    } else {
                        outTokenMatrix.addText("STATUS fail player_offline");
                    }
                } else {
                    outTokenMatrix.addText("STATUS fail player_not_exists");
                }
            }

            // Just repassing...
            else if (inTokenMatrix[0][0] == "BEGINREQACK" || inTokenMatrix[0][0] == "BEGINRESPACK") {
                int i = inTokenMatrix.findLineIndex("FROM");
                std::string fromUsername = inTokenMatrix[i][1];
                auto it = allPlayers.find(fromUsername);

                if (it != allPlayers.end()) {
                    Player& player = it->second;
                    int otherThreadPipeWriter = player.getPipeWriter();
                    Communication::writeToPipe(otherThreadPipeWriter, inTokenMatrix.str());
                    continue;
                }
            }

            // Message received when two clients have sucessfuly made a connection (a match has begun)
            else if (inTokenMatrix[0][0] == "START") {
                int i = inTokenMatrix.findLineIndex("USERNAME");
                std::string username = inTokenMatrix[i][1];

                std::string againstUsername = inTokenMatrix[0][1];

                Player& player = allPlayers.find(username)->second;
                Player& againstPlayer = allPlayers.find(againstUsername)->second;
                player.startGame(againstUsername);

                logger.matchStarted(player.getUsername(), player.getIpAdress(),
                                    againstPlayer.getUsername(), againstPlayer.getIpAdress());
                continue;
            }

            // Message received after two clients have finished their game
            else if (inTokenMatrix[0][0] == "FINISH") {
                int i = inTokenMatrix.findLineIndex("USERNAME");
                std::string username = inTokenMatrix[i][1];

                std::string result = inTokenMatrix[0][1];

                Player& player = allPlayers.find(username)->second;
                Player& againstPlayer = allPlayers.find(player.getEnemyUsername())->second;
                player.endGame();

                if (result == "win") {
                    player.setScore(player.getScore() + 2);
                    player.saveToFile(storedPlayersDir);

                    logger.matchEnded(player.getUsername(), player.getIpAdress(),
                                      againstPlayer.getUsername(), againstPlayer.getIpAdress(),
                                      player.getUsername());

                } else if (result == "draw") {
                    player.setScore(player.getScore() + 1);
                    player.saveToFile(storedPlayersDir);

                    logger.matchEnded(player.getUsername(), player.getIpAdress(),
                                      againstPlayer.getUsername(), againstPlayer.getIpAdress(),
                                      "No one (draw)");
                }

                continue;
            }

            else if (inTokenMatrix[0][0] == "EXIT") {
                int i = inTokenMatrix.findLineIndex("IP");
                std::string ip = inTokenMatrix[i][1];
                logger.clientDisconnected(ip);
                outTokenMatrix.addText("EXITACK");
            }

            else if (inTokenMatrix[0][0] == "TERMINATED") {
                int i = inTokenMatrix.findLineIndex("IP");
                std::string ip = inTokenMatrix[i][1];
                logger.unexpectedClientDisconnect(ip);
            }

            if (threadPipeWriter != -1) {
                Communication::writeToPipe(threadPipeWriter, outTokenMatrix.str());
            }
        }

        // If write side of main pipe has been closed, end this thread
        if (!messages.first) {
            close(mainPipeReader);
            break;
        }
    }
}
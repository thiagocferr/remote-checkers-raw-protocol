
#include <filesystem>
#include <unordered_map>
#include <mutex>

#include "Player.hpp"

// Write side of main thread pipe that receives messages from all threads
extern int mainPipeWriter;
extern std::mutex pipeWriterMutex;

// While user is not logged in, the only way for the main thread to know where to reply its message
//  is by association the thread's id to a write side of a pipe. When user logs in, this information will
//  be available inside the Player's object
extern std::unordered_map<std::string, int> threadIdToPipe;
extern std::mutex threadIdToPipeMutex;

extern std::unordered_map<std::string, Player> allPlayers;

void threadTask(int connfd, std::string ipAddress);

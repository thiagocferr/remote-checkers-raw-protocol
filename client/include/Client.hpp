#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <cstddef>
#include <unordered_map>
#include <queue>
#include <chrono>

#include "Tictactoe.hpp"


// In which status the client currently is. Note that ON_MATCH also implies that the
// client is logged
enum class ClientStatus {
    NOT_LOGGED,
    LOGGED,
    WAITING_MATCH,
    ON_MATCH
};

// Stores info about the opponent of this client on the tic-tac-toe game
struct RivalClientInfo {


};

struct GameInfo {

    // Current status of the game's board
    Tictactoe gameBoard;

    // Is this client's turn?
    bool isTurn;

    // Ath this game, which piece (X or O) this client is
    GamePiece clientPiece;

    // Contains the last 3 latencies between players during a game
    std::deque<int> latencies;
};

class Client {
    private:
        // The current status of the client. Dictates which commands can be used or not
        enum ClientStatus status;

        // The username of the player this client is currently logged as
        std::string loggedUsername;

        // Keep track of usernames of players that this client sent invites to or received from]
        // (used during matchimaking)
        std::string inviteFrom;
        std::string inviteTo;

        // The name of the advsersary (if on a match)
        std::string rivalUsername;

        // Gather information about a game of tic-tac-toe between two clients
        GameInfo game;

        // Mark if user client wants to close the application
        bool timeToExit;
    public:

        Client();

        void setStatus(ClientStatus newStatus);
        ClientStatus getStatus();
        std::string getStatusString();

        void setLoggedUsername(std::string username);
        std::string getLoggedUsername();

        // Used during matchmaking
        void setInviteFrom(std::string inviteFrom);
        std::string getInviteFrom();
        void setInviteTo(std::string inviteTo);
        std::string getInviteTo();

        void setRivalUsername(std::string username);
        std::string getRivalUsername();

        void setGamePiece(GamePiece piece);
        GamePiece getGamePiece();

        void setOnTurn();
        void setOffTurn();
        bool isOnTurn();

        bool canMakePlay(int row, int col);
        // After making a move, return the current game's status
        GameStatus makePlay(int row, int col);
        GameStatus receivePlay(int row, int col);
        GameStatus getCurrentGameStatus();

        std::string getBoardVisual();
        void storeLatency(int ms);
        std::deque<int> getLatencies();

        bool isOnGame();
        void endGame();

        // Used when user inputs 'exit' command
        void markForExit();
        bool isTimeToExit();
};

#endif


#include "Client.hpp"

#include <random>

#include "MessageTypes.hpp"

static std::unordered_map<ClientStatus, std::string> clientStatusToString {
    {ClientStatus::NOT_LOGGED, "not logged"},
    {ClientStatus::LOGGED, "already logged"},
    {ClientStatus::WAITING_MATCH, "waiting for a match"},
    {ClientStatus::ON_MATCH, "on a match"}
};

Client::Client() {
    this->status = ClientStatus::NOT_LOGGED;
    this->timeToExit = false;
}

void Client::setStatus(ClientStatus newStatus) {
    this->status = newStatus;
}

ClientStatus Client::getStatus() {
    return this->status;
}

std::string Client::getStatusString() {
    auto t = clientStatusToString.find(this->status);
    if (t == clientStatusToString.end())
        return "";
    return t->second;
}

void Client::setLoggedUsername(std::string username) {
    this->loggedUsername = username;
}
std::string Client::getLoggedUsername() {
    return this->loggedUsername;
}



void Client::setInviteFrom(std::string inviteFrom) {
    this->inviteFrom = inviteFrom;
}
std::string Client::getInviteFrom() {
    return this->inviteFrom;
}
void Client::setInviteTo(std::string inviteTo) {
    this->inviteTo = inviteTo;
}
std::string Client::getInviteTo() {
    return this->inviteTo;
}



void Client::setRivalUsername(std::string username) {
    this->rivalUsername = username;
}
std::string Client::getRivalUsername() {
    return this->rivalUsername;
}

void Client::setGamePiece(GamePiece piece) {
    this->game.clientPiece = piece;
}
GamePiece Client::getGamePiece() {
    return this->game.clientPiece;
}



void Client::setOnTurn() {
    this->game.isTurn = true;
}
void Client::setOffTurn() {
    this->game.isTurn = false;
}
bool Client::isOnTurn() {
    return this->game.isTurn;
}

// Check if position is already marked by some piece
bool Client::canMakePlay(int row, int col) {
    return this->game.gameBoard.canSetPlay(row, col);
}

GameStatus Client::makePlay(int row, int col) {
    this->game.gameBoard.setPlay(row, col, this->game.clientPiece);
    return this->getCurrentGameStatus();
}

GameStatus Client::receivePlay(int row, int col) {
    GamePiece oppositePiece =
        (this->getGamePiece() == GamePiece::X ? GamePiece::O : GamePiece::X);

    this->game.gameBoard.setPlay(row, col, oppositePiece);
    return this->getCurrentGameStatus();
}

GameStatus Client::getCurrentGameStatus() {
    return this->game.gameBoard.checkWin();
}


std::string Client::getBoardVisual() {
    return this->game.gameBoard.getBoard();
}

void Client::storeLatency(int ms) {
    if (this->game.latencies.size() == 3)
        this->game.latencies.pop_front();
    this->game.latencies.push_back(ms);
}

std::deque<int> Client::getLatencies() {
    return this->game.latencies;
}




bool Client::isOnGame(){
    return this->rivalUsername == "" ? false : true;
}

void Client::endGame() {
    if (this->status == ClientStatus::ON_MATCH) {
        this->status = ClientStatus::LOGGED;

        this->setInviteFrom("");
        this->setInviteTo("");

        this->setRivalUsername("");
        this->setGamePiece(GamePiece::Empty);
        this->setOffTurn();
        this->game.latencies = std::deque<int>();
        this->game.gameBoard = Tictactoe();
    }
}

void Client::markForExit() {
    this->timeToExit = true;
}
bool Client::isTimeToExit() {
    return this->timeToExit;
}
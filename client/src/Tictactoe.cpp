#include "Tictactoe.hpp"
#include <string>

Tictactoe::Tictactoe() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            this->game[i][j] = GamePiece::Empty;
        }
    }
}

std::string Tictactoe::getStateMessage() {
    std::string message = "";
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            switch(this->game[i][j]) {
                case GamePiece::Empty:
                    message += "_";
                    break;
                case GamePiece::O:
                    message += "o";
                    break;
                case GamePiece::X:
                    message += "x";
                    break;
            }
        }
    }
    return message;
}

bool Tictactoe::canSetPlay(int row, int col) {
    return this->game[row][col] == GamePiece::Empty;
}

bool Tictactoe::setPlay(int row, int col, GamePiece piece) {

    if ( this->game[row][col] != GamePiece::Empty )
        return false;
    this->game[row][col] = piece;
    return true;
}

GameStatus Tictactoe::checkWin() {
    // check columns
    for (int i = 0; i < 3; i++) {
        if (this->game[0][i] == this->game[1][i] && this->game[1][i] == this->game[2][i]) {
            GamePiece piece = this->game[0][i];
            if (piece == GamePiece::O)
                return GameStatus::O_Winner;
            else if (piece == GamePiece::X)
                return GameStatus::X_Winner;
        }
    }

    // Checking rows
    for (int i = 0; i < 3; i++) {
        if (this->game[i][0] == this->game[i][1] && this->game[i][1] == this->game[i][2]) {
            GamePiece piece = this->game[i][0];
            if (piece == GamePiece::O)
                return GameStatus::O_Winner;
            else if (piece == GamePiece::X)
                return GameStatus::X_Winner;
        }
    }

    // Checking diagonals
    GamePiece piece;
    if (((piece = this->game[0][0]) == this->game[1][1] && this->game[1][1] == this->game[2][2]) ||
        ((piece = this->game[2][0]) == this->game[1][1] && this->game[1][1] == this->game[0][2])) {

            if (piece == GamePiece::O)
                return GameStatus::O_Winner;
            else if (piece == GamePiece::X)
                return GameStatus::X_Winner;
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (this->game[i][j] == GamePiece::Empty) {
                return GameStatus::In_Progress;
            }
        }
    }
    return GameStatus::Draw;
}

std::string Tictactoe::getBoard() {
    std::string board;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            switch(this->game[i][j]) {
                case GamePiece::Empty:
                    board += "   ";
                    break;
                case GamePiece::O:
                    board += " o ";
                    break;
                case GamePiece::X:
                    board += " x ";
                    break;
            }
            if (j != 2) board += "|";
        }
        board += "\n";
        if (i != 2) board += "-----------\n";
    }
    return board;
}
#ifndef TICTACTOE_HPP
#define TICTACTOE_HPP

#include <string>

enum class GamePiece {
    Empty,
    X,
    O,
};

enum class GameStatus {
    In_Progress,
    X_Winner,
    O_Winner,
    Draw
};

class Tictactoe {
    private:
        GamePiece game[3][3];
    public:
        Tictactoe();

        // Stringfy board (empty spaces are represented by "_", spaces occupied by X is represented by "x"
        //  and spaces occupied by O, "o")
        std::string getStateMessage();

        // Check if it's possible to place a piece at the specified position (i.e. there's no player
        //  piece there yet)
        bool canSetPlay(int row, int col);

        // Put a piece on the board at specific row and col and return if it was performed or not
        bool setPlay(int row, int col, GamePiece piece);

        // Get printable board
        std::string getBoard();

        // 0: no winner yet; 1: player X won; 2: player O won; 3: draw
        GameStatus checkWin();
};

#endif
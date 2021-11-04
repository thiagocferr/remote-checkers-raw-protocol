
#include "ParserClientMessage.hpp"

#include <iostream>
#include <any>

#include "Tokenizer.hpp"

namespace Parser {

    namespace {

        std::vector<std::string> parseSend(TokenMatrix& serverMessage, Client& client) {

            int row, col;
            row = std::stoi(serverMessage[0][1]);
            col = std::stoi(serverMessage[0][2]);

            GameStatus gameStatus = client.receivePlay(row, col);

            std::cout << "Updated game board:" << std::endl;
            std::cout << client.getBoardVisual() << std::endl;
            // std::cout <<  std::endl;

            client.setOnTurn();
            std::cout << "Now it's your turn!" << std::endl;

            if ((gameStatus == GameStatus::X_Winner && client.getGamePiece() == GamePiece::X) ||
                (gameStatus == GameStatus::O_Winner && client.getGamePiece() == GamePiece::O)) {

                std::cout << "____Match ended: YOU WIN!____" << std::endl;
                return {"FINISH win"};
            }
            else if ((gameStatus == GameStatus::O_Winner && client.getGamePiece() == GamePiece::X) ||
                    (gameStatus == GameStatus::X_Winner && client.getGamePiece() == GamePiece::O)) {

                std::cout << "____Match ended: YOU LOST!____" << std::endl;
                return {"FINISH lost"};
            }
            else if (gameStatus == GameStatus::Draw) {

                std::cout << "____Match ended: IT WAS A DRAW!____" << std::endl;
                return {"FINISH draw"};
            }

            return {""};
        }

        std::vector<std::string> parseEnd([[maybe_unused]]TokenMatrix& serverMessage, [[maybe_unused]]Client& client) {

            std::cout << "You opponent \"" + client.getRivalUsername() + "\" has abandoned the match!" << std::endl;
            std::cout << "You win!" << std::endl;

            client.endGame();
            return {"FINISH win"};
        }

        // MUST BE DEFINED AT THE END OF UNNAMED NAMESPACE
        std::unordered_map<std::string, std::any> messageTypeToParserFunction {
            {"SEND", parseSend},
            {"END" , parseEnd},
        };
    }


    std::vector<std::string> parseClientMessage(const std::string& message, Client& client) {
        TokenMatrix parsedMessage(message);
        return std::any_cast<std::vector<std::string> (*) (TokenMatrix&, Client&)>
            (messageTypeToParserFunction[parsedMessage[0][0]]) (parsedMessage, client);
    }
}
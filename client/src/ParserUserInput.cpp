
#include "ParserUserInput.hpp"

#include <string>
#include <cstddef>
#include <iostream>
#include <vector>
#include <any>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <stdexcept>


#include "Client.hpp"
#include "MessageTypes.hpp"
#include "Tictactoe.hpp"
#include "Tokenizer.hpp"

namespace Parser {
    namespace {

        // Given a number of expected lines and the number of expected tokens in each line, check if input
        // follows it
        bool isNumOfParamsCorrect(unsigned int numLines, std::vector<unsigned int> numTokens, TokenMatrix& userInput) {
            if (userInput.size() != numLines)
                return false;
            for (unsigned int i = 0; i < numLines; i++)
                if (userInput[i].size() != numTokens[i])
                    return false;
            return true;
        }

        // Login System
        std::vector<std::string> parseLoginSystem(TokenMatrix& userInput, [[maybe_unused]] Client& client) {

            // Check for correct number of tokens (and lines)
            if (!isNumOfParamsCorrect(1, {3}, userInput)) {
                std::cout << "Invalid number of parameters!" << std::endl;
                return {""};
            }

            return {userInput.str()};
        }

        // Game commands
        std::vector<std::string> parseExit([[maybe_unused]] TokenMatrix& userInput, [[maybe_unused]] Client& client) {
            return {"EXIT"};
        }

        std::vector<std::string> parseSend(TokenMatrix& userInput, Client& client) {

            // Check for correct number of tokens (and lines)
            if (!isNumOfParamsCorrect(1, {3}, userInput)) {
                std::cout << "Invalid number of parameters!" << std::endl;
                return {""};
            }

            // Check if parameters are integers inside range
            int row, col;
            try {
                row = std::stoi(userInput[0][1]);
                col = std::stoi(userInput[0][2]);

                // Check if parameters are inside game boundries
                if (row < 0 || row > 2 || col < 0 || col > 2)
                    throw std::out_of_range("Out of board range (0 - 2)");

            } catch (std::invalid_argument&) {
                std::cout << "'send' parameters must be integers" << std::endl;
                return {""};
            } catch (std::out_of_range&) {
                std::cout << "'send' parameters must be integers between 0 and 2" << std::endl;
                return {""};
            }

            if (!client.isOnTurn()) {
                std::cout << "It's not your turn yet! Wait for other player's move" << std::endl;
                std::cout << "To abandon match, send command 'end'" << std::endl;
                return {""};
            }

            if (!client.canMakePlay(row, col)) {
                std::cout << "ILLEGAL PLAY: position at row " + std::to_string(row) + ", column " + std::to_string(col) + " is already occupied!" << std::endl;
                std::cout << "Select an empty position!" << std::endl;
                return {""};
            }

            GameStatus gameStatus = client.makePlay(row, col);
            std::cout << "Updated game board:" << std::endl;
            std::cout << client.getBoardVisual() << std::endl;
            // std::cout <<  std::endl;

            client.setOffTurn();
            std::cout << "Now it's " + client.getRivalUsername() + "'s turn!" << std::endl;

            if ((gameStatus == GameStatus::X_Winner && client.getGamePiece() == GamePiece::X) ||
                (gameStatus == GameStatus::O_Winner && client.getGamePiece() == GamePiece::O)) {

                std::cout << "____Match ended: YOU WIN!____" << std::endl;
                return {userInput.str(), "FINISH win"};
            }
            else if ((gameStatus == GameStatus::O_Winner && client.getGamePiece() == GamePiece::X) ||
                    (gameStatus == GameStatus::X_Winner && client.getGamePiece() == GamePiece::O)) {

                std::cout << "____Match ended: YOU LOST!____" << std::endl;
                return {userInput.str(), "FINISH lost"};
            }
            else if (gameStatus == GameStatus::Draw) {

                std::cout << "____Match ended: IT WAS A DRAW!____" << std::endl;
                return {userInput.str(), "FINISH draw"};
            }

            return {userInput.str()};
        }


        std::vector<std::string> parseBegin(TokenMatrix& userInput, Client& client) {

            if (!isNumOfParamsCorrect(1, {2}, userInput)) {
                std::cout << "Invalid number of parameters!" << std::endl;
                return {""};
            }

            client.setStatus(ClientStatus::WAITING_MATCH);
            client.setInviteTo(userInput[0][1]);

            TokenMatrix out("BEGINREQ " + userInput[0][1]);
            out.addText("FROM " + client.getLoggedUsername());

            return {out.str()};
        }

        std::vector<std::string> parseCancel(TokenMatrix& userInput, Client& client) {
            if (!isNumOfParamsCorrect(1, {1}, userInput)) {
                std::cout << "Invalid number of parameters!" << std::endl;
                return {""};
            }

            if (client.getInviteTo() == "") {
                std::cout << "Invalid command: not waiting for a match invitation response" << std::endl;
                return {""};
            }

            std::string cancelInviteTo = client.getInviteTo();
            client.setInviteTo("");
            client.setStatus(ClientStatus::LOGGED);
            return {"CANCEL " + cancelInviteTo};
        }

        std::vector<std::string> parseInvite(TokenMatrix& userInput, Client& client) {

            if (!isNumOfParamsCorrect(1, {2}, userInput)) {
                std::cout << "Invalid number of parameters!" << std::endl;
                return {""};
            }

            std::string decision = userInput[0][1];

            if (decision != "accept" && decision != "reject") {
                std::cout << "Incorrect use of commands: Must be 'invite accept' OR 'invite reject'" << std::endl;
                return {""};
            }

            if (client.getInviteFrom() == "") {
                std::cout << "Invalid command: no match invitation received!" << std::endl;
                return {""};
            }

            if (decision == "accept")
                client.setStatus(ClientStatus::WAITING_MATCH);

            TokenMatrix out("BEGINRESP " + client.getInviteFrom() + " " + decision);
            out.addText("FROM " + client.getLoggedUsername());

            return {out.str()};
        }

        std::vector<std::string> parseEnd(TokenMatrix& userInput, Client& client) {
            std::cout << "You abandoned the match against \"" + client.getRivalUsername() + "\"!" << std::endl;
            std::cout << "You lost!" << std::endl;

            client.endGame();
            return {userInput.str(), "FINISH lost"};
        }

        // Client Side
        std::vector<std::string> parseDelay(TokenMatrix& userInput, Client& client) {

            if (!isNumOfParamsCorrect(1, {1}, userInput)) {
                std::cout << "Invalid number of parameters!" << std::endl;
                return {""};
            }

            std::string message = "Last 3 latencies: ";

            auto latencies = client.getLatencies();
            for (int latency: latencies)
                std::cout << std::to_string(latency) <<  "ms, ";
            std::cout << std::endl;

            return {""};
        }

        // Common commands
        std::vector<std::string> parseCommon(TokenMatrix& userInput, [[maybe_unused]]Client& client) {

            if (!isNumOfParamsCorrect(1, {1}, userInput)) {
                std::cout << "Invalid number of parameters!" << std::endl;
                return {""};
            }

            return {userInput[0][0]};
        }

        // MUST BE DEFINED AT THE END OF UNNAMED NAMESPACE
        std::unordered_map<std::string, std::any> messageTypeToParserFunction {
            {"adduser", parseLoginSystem},
            {"passwd" , parseLoginSystem},
            {"login"  , parseLoginSystem},

            {"logout" , parseCommon},
            {"leaders", parseCommon},
            {"list"   , parseCommon},

            {"begin"  , parseBegin},
            {"cancel" , parseCancel},
            {"send"   , parseSend},
            {"exit"   , parseExit},
            {"invite", parseInvite},
            {"end"    , parseEnd},

            {"delay"  , parseDelay}
        };
    }


    std::vector<std::string> parseUserInput(const std::string& userInput, Client& client) {

        std::string command = MessageTypes::getMessageTypeSubstring(userInput);
        std::set<ClientStatus> allowedStates = MessageTypes::getAllowedStatesForCommand(userInput);

        // Check if command exists
        if (allowedStates.empty()) {
            std::cout << "Command '" << command << "' is not valid" << std::endl;
            return {""};
        }

        // Check if command can be executed in Client's current state
        if (allowedStates.find(client.getStatus()) == allowedStates.end()) {
            std::cout << "You cannot use command '" << command << "' while "
                << client.getStatusString() << std::endl;
            return {""};
        }

        TokenMatrix parsedInput(userInput);

        // Command Token to Uppercase (for message passing) (in this case, the first word)
        std::transform(parsedInput[0][0].begin(), parsedInput[0][0].end(),parsedInput[0][0].begin(), ::toupper);

        // Associate a certain command with a function that will create a Message and, possibly, a side-effect on the
        // Client object
        std::vector<std::string> returnVal = std::any_cast<std::vector<std::string> (*) (TokenMatrix&, Client&)>
            (messageTypeToParserFunction[command]) (parsedInput, client);

        return returnVal;
    }
}
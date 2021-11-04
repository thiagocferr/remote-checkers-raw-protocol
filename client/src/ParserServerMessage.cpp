
#include "ParserServerMessage.hpp"

#include <iostream>
#include <string>
#include <random>

#include "Client.hpp"
#include "MessageTypes.hpp"
#include "Tictactoe.hpp"
#include "Tokenizer.hpp"

namespace Parser {

    namespace {

        // Setting up randomness engine
        std::random_device rd;
        std::mt19937 gen(rd());
        std::bernoulli_distribution dist(0.5);

        bool getRandomBool() {
            return dist(gen);
        }


        std::vector<std::string> parseAdduserAck(TokenMatrix& serverMessage, [[maybe_unused]]Client& client) {

            int i = serverMessage.findLineIndex("STATUS");

            std::string user = serverMessage[0][1];
            std::string status = serverMessage[i][1];

            if (status == "ok") {
                std::cout << "User '" + user + "' was created!" << std::endl;
            } else {
                std::cout << "Failed to create user '" + user + "': Username already exists!" << std::endl;
            }

            return {""};
        }

        std::vector<std::string> parseLoginAck(TokenMatrix& serverMessage, Client& client) {

            int i = serverMessage.findLineIndex("STATUS");

            std::string user = serverMessage[0][1];
            std::string status = serverMessage[i][1];

            if (status == "ok") {
                client.setStatus(ClientStatus::LOGGED);
                client.setLoggedUsername(user);
                std::cout << "Logged in as user '" + user + "'!" << std::endl;
            } else {

                std::string reason = serverMessage[i][2];
                std::cout << "Logging failed: " + reason << std::endl;
            }

            return {""};
        }

        std::vector<std::string> parsePasswdAck(TokenMatrix& serverMessage, [[maybe_unused]]Client& client) {
            int i = serverMessage.findLineIndex("STATUS");
            std::string status = serverMessage[i][1];

            if (status == "ok") {
                std::cout << "Password successfully changed!" << std::endl;
            } else {
                std::cout << "Failed to change password: Wrong password!" << std::endl;
            }

            return {""};
        }

        std::vector<std::string> parseLeaders(TokenMatrix& serverMessage, [[maybe_unused]]Client& client) {

            std::cout << "Leaderboard:" << std::endl;
            std::cout << "Username \t Score" << std::endl;
            for (unsigned int i = 1, j = 1; i < serverMessage[0].size(); i+=2, j++) {
                std::string username = serverMessage[0][i];
                std::string score = serverMessage[0][i + 1];

                std::cout << std::to_string(j) + ". " + username + "\t " + score << std::endl;
            }

            return {""};
        }

        std::vector<std::string> parseList(TokenMatrix& serverMessage, [[maybe_unused]]Client& client) {

            std::cout << "List of players:" << std::endl;
            std::cout << "Username \t On match" << std::endl;
            for (unsigned int i = 1; i < serverMessage[0].size(); i+=2) {
                std::string username = serverMessage[0][i];
                std::string isOnMatch = serverMessage[0][i + 1];

                std::cout << std::to_string(i) + ". " + username + "\t " + isOnMatch << std::endl;
            }

            return {""};
        }

        std::vector<std::string> parseLogoutAck(TokenMatrix& serverMessage, Client& client) {

            std::string user = serverMessage[0][1];
            client.setStatus(ClientStatus::NOT_LOGGED);
            client.setLoggedUsername("");
            std::cout << "Logged out as user '" + user + "'!" << std::endl;

            return {""};
        }

        std::vector<std::string> parseCancel(TokenMatrix& serverMessage, Client& client) {
            int i  = serverMessage.findLineIndex("FROM");
            std::string whoIsCancelling = serverMessage[i][1];
            std::string whoIsBeingCancelled = serverMessage[0][1];

            if (client.getInviteFrom() == whoIsCancelling) {
                client.setInviteFrom("");
                std::cout << "User \"" + whoIsCancelling + "\"'s invitation has been cancelled! " << std::endl;
            }

            return {""};
        }

        // This one is for when client is the one who SENT a match request and is receiving a check
        // (if invitation could be sent or not)
        std::vector<std::string> parseBeginReqAck(TokenMatrix& serverMessage, Client& client) {
            int i = serverMessage.findLineIndex("STATUS");
            std::string requestedUsername = serverMessage[0][1];
            if (serverMessage[i][1] == "ok") {
                std::cout << "Match request sent to '" + requestedUsername + "'..." << std::endl;
                std::cout << "Type 'cancel' to cancel match'" << std::endl;
            } else {
                client.setStatus(ClientStatus::LOGGED);
                client.setInviteTo("");
                std::string reason = serverMessage[i][2];
                std::cout << "Match request to '" + requestedUsername + "' failed: " + reason << std::endl;
            }

            return {""};
        }

        // This one is for when the client RECEIVED a match request. So, for now, wait for client to accept or not
        // (via command line)
        std::vector<std::string> parseBeginReq(TokenMatrix& serverMessage, Client& client) {

            int i = serverMessage.findLineIndex("FROM");

            std::string fromUsername = serverMessage[i][1];
            std::string toUsername = serverMessage[0][1];

            TokenMatrix outTokenMatrix;
            outTokenMatrix.addText("BEGINREQACK " + toUsername);
            outTokenMatrix.addText("FROM " + fromUsername);

            ClientStatus status = client.getStatus();
            if (status == ClientStatus::WAITING_MATCH) {
                outTokenMatrix.addText("STATUS fail player_waiting_for_invitation_response");
                return {outTokenMatrix.str()};
            }

            if (status == ClientStatus::ON_MATCH) {
                outTokenMatrix.addText("STATUS fail player_in_a_match");
                return {outTokenMatrix.str()};
            }

            if (client.getInviteFrom() != "" && client.getInviteFrom() != fromUsername) {
                outTokenMatrix.addText("STATUS fail player_aready_invited");
                return {outTokenMatrix.str()};
            }

            client.setInviteFrom(fromUsername);

            std::cout << "____Match request received from '" + fromUsername + "'____" << std::endl;
            std::cout << "For accepting, send command 'invite accept'. For rejecting, command 'invite reject'" << std::endl;

            outTokenMatrix.addText("STATUS ok");
            return {outTokenMatrix.str()};
        }

        // When a client accept or declines a match invite, it sends a "BEGINRESP" to the server with their answer.
        // Server responds with a "BEGINRESPACK" for letting the client know if the response could be sent
        std::vector<std::string> parseBeginRespAck(TokenMatrix& serverMessage, Client& client) {


            int i = serverMessage.findLineIndex("STATUS");
            std::string respondedUsername = serverMessage[0][1];
            if (serverMessage[i][1] == "ok") {
                client.setRivalUsername(client.getInviteFrom());
                client.setStatus(ClientStatus::ON_MATCH);

                client.setInviteFrom("");
                client.setInviteTo("");

                // Client's game piece
                int j = serverMessage.findLineIndex("PIECE");
                if (serverMessage[j][1] == "o")
                    client.setGamePiece(GamePiece::O);
                else
                    client.setGamePiece(GamePiece::X);

                // If client starts the match or not
                int k = serverMessage.findLineIndex("FIRST");
                if (serverMessage[k][1] == "y")
                    client.setOnTurn();
                else
                    client.setOffTurn();

                GamePiece choosenPiece = client.getGamePiece();
                std::cout << "____Starting match____" << std::endl;
                std::cout << std::string("Your starting piece is ") + (choosenPiece == GamePiece::X ? "X" : "O") << std::endl;

                if (client.isOnTurn()) {
                    std::cout << "It's your turn!" << std::endl;
                } else {
                    std::cout << "It's " + client.getRivalUsername() + "'s turn!" << std::endl;
                }

            } else {
                client.setStatus(ClientStatus::LOGGED);
                client.setInviteFrom("");
                std::string reason = serverMessage[i][2];
                std::cout << "Match response to '" + respondedUsername + "' failed: " + reason << std::endl;
            }

            return {""};
        }

        // This is received at the end of matchmaking, where both sides agreed to start a match
        std::vector<std::string> parseBeginResp(TokenMatrix& serverMessage, Client& client) {

            int i = serverMessage.findLineIndex("FROM");

            std::string fromUsername = serverMessage[i][1];
            std::string toUsername = serverMessage[0][1];
            std::string response = serverMessage[0][2];

            TokenMatrix outTokenMatrix;
            outTokenMatrix.addText("BEGINRESPACK " + toUsername + " " + response);
            outTokenMatrix.addText("FROM " + fromUsername);

            ClientStatus status = client.getStatus();
            if (client.getInviteTo() != fromUsername || status != ClientStatus::WAITING_MATCH) {


                outTokenMatrix.addText("STATUS fail player_not_available_for_match");
                return {outTokenMatrix.str()};
            }

            if (response == "accept") {

                outTokenMatrix.addText("STATUS ok");

                client.setRivalUsername(client.getInviteTo());
                client.setStatus(ClientStatus::ON_MATCH);

                client.setInviteTo("");
                client.setInviteFrom("");

                // Decide starting pieces (random)
                if (getRandomBool()) {
                    client.setGamePiece(GamePiece::X);
                    outTokenMatrix.addText("PIECE o");
                } else {
                    client.setGamePiece(GamePiece::O);
                    outTokenMatrix.addText("PIECE x");
                }

                // Deciding how starts the game (random)
                if (getRandomBool()) {
                    client.setOnTurn();
                    outTokenMatrix.addText("FIRST n");
                } else {
                    client.setOffTurn();
                    outTokenMatrix.addText("FIRST y");
                }

                GamePiece choosenPiece = client.getGamePiece();
                std::cout << "____Player '" + fromUsername + "' accepted the invite____" << std::endl;
                std::cout << std::string("Your starting piece is ") + (choosenPiece == GamePiece::X ? "X" : "O") << std::endl;;
                return {outTokenMatrix.str()};

            // If a player received a response to their invitation and it was a rejection, there's no need for
            //  the other player to know the response
            } else if (response == "reject") {
                client.setStatus(ClientStatus::LOGGED);
                client.setInviteTo("");
                std::cout << "____Player '" + fromUsername + "' rejected the invite____" << std::endl;
            }

            return {""};
        }

        std::vector<std::string> parseExitAck([[maybe_unused]] TokenMatrix& serverMessage, Client& client) {
            client.markForExit();
            return {""};
        }


        // MUST BE DEFINED AT THE END OF UNNAMED NAMESPACE
        std::unordered_map<std::string, std::any> messageTypeToParserFunction {
            {"ADDUSERACK", parseAdduserAck},
            {"LOGINACK" , parseLoginAck},
            {"PASSWDACK"  , parsePasswdAck},
            {"LEADERS", parseLeaders},
            {"LIST", parseList},
            {"LOGOUTACK", parseLogoutAck},
            {"CANCEL", parseCancel}, // Receiving a cancellation of invitation, if any
            {"BEGINREQACK", parseBeginReqAck}, // Received by user how SENT the invitation
            {"BEGINREQ", parseBeginReq}, // Received by user who RECEIVED the invitation
            {"BEGINRESPACK", parseBeginRespAck},
            {"BEGINRESP", parseBeginResp},
            {"EXITACK", parseExitAck}
        };
    }

    std::vector<std::string> parseServerMessage(const std::string& message, Client& client) {


        TokenMatrix parsedMessage(message);
        return std::any_cast<std::vector<std::string> (*) (TokenMatrix&, Client&)>
            (messageTypeToParserFunction[parsedMessage[0][0]]) (parsedMessage, client);
    }
}
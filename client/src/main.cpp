
#include <iostream>
#include <cstdlib>
#include <ostream>

#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>

#include "Client.hpp"
#include "ParserUserInput.hpp"
#include "ParserServerMessage.hpp"
#include "ParserClientMessage.hpp"
#include "Tictactoe.hpp"

#include "Communication.hpp"
#include "Tokenizer.hpp"

#define NI_MAXSERV    32
#define NI_MAXHOST  1025

int createGameListingSocket() {
    int listenfd;
    struct sockaddr_in clientaddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    // where socketfd is the socket you want to make non-blocking
    fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL, 0) | O_NONBLOCK);

    bzero(&clientaddr, sizeof(clientaddr));
    clientaddr.sin_family      = AF_INET;
    clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientaddr.sin_port        = htons(0);

    if (bind(listenfd, (struct sockaddr *)&clientaddr, sizeof(clientaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    if (listen(listenfd, 1) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    return listenfd;
}


int connectToServer(const char address[], int port) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr,"socket error :( \n");
        exit(3);
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &servaddr.sin_addr) <= 0) {
        fprintf(stderr,"inet_pton error for %s :(\n", address);
        exit(4);
    }


    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr,"connect error :(\n");
        exit(5);
    }

    return sockfd;
}

//
std::string getExternalIPAddress() {
    struct ifaddrs *ifaddr;
    int family;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(7);
    }

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        if (std::string(ifa->ifa_name) == "lo")
            continue;

        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) {
            getnameinfo(
                ifa->ifa_addr,
                sizeof(struct sockaddr_in),
                host, NI_MAXHOST,
                NULL, 0, NI_NUMERICHOST);

            std::string ip;
            if ((ip = std::string(host)) != "127.0.0.1") {
                return ip;
            }
        }
    }

    return "";
}

void closingGame(struct pollfd *pfds, unsigned int* nfds, int* clientConnFd, Client& client) {
    close(pfds[2].fd);

    *nfds = 2;
    pfds[2].fd = -1;
    *clientConnFd = -1;

    client.endGame();
}

int main(int argc, char * argv[]) {

    if (argc != 3) {
        fprintf(stderr,"Use: %s <Server IP><Port>\n",argv[0]);
        fprintf(stderr,"This is going to a client and connect it to a server on IP <IP>, port <PORT>\n");
        exit(1);
    }

    int port;
    if ((port = strtol(argv[2], NULL, 10)) == 0) {
        fprintf(stderr,"port conversion error for %s :( \n", argv[2]);
        exit(2);
    }

    // Connection to server and, when on a match, to the other client
    int connfd = connectToServer(argv[1], port);
    int clientConnFd;

    // Polling preparation
    struct pollfd *pfds;
    unsigned int nfds = 2;
    if ((pfds = (struct pollfd *) calloc(3, sizeof(struct pollfd))) == NULL) {
        perror("Malloc array of poll file descriptors");
        exit(6);
    }

    pfds[0].fd = 0; // This is stdin
    pfds[0].events = POLLIN;
    pfds[1].fd = connfd; // This is the server connection
    pfds[1].events = POLLIN;

    bool isServerSocketOk = true;
    bool isAcceptingConn = false;

    Client client;

    std::cout << "JogoDaVelha> " << std::flush;;
    // Main loop
    while (isServerSocketOk) {
        std::string user_input;

        if (poll(pfds, nfds, -1) == -1) {
            std::cout << "THREAD FATAL ERROR: polling failed";
            break;
        }

        // Received input from USER
        if (pfds[0].revents & POLLIN) {
            std::getline(std::cin, user_input);
            if (user_input.empty())
                continue;

            auto messages = Parser::parseUserInput(user_input, client);
            for (std::string message : messages) {
                if (message == "") continue;

                TokenMatrix inTokenMatrix(message);
                std::string messageType = inTokenMatrix[0][0];
                if (messageType == "SEND" || messageType == "END" || messageType == "PING" ||
                    messageType == "PONG") {
                    Communication::writeToSocket(clientConnFd, message);
                } else {
                    isServerSocketOk = Communication::writeToSocket(connfd, message);
                }

                // Ending game (cleaning client's internals, closing FD and reajusting poll parameters)
                //  after doing a play
                if (messageType == "FINISH") {
                    closingGame(pfds, &nfds, &clientConnFd, client);
                }
            }
        }

        // RECEIVED FROM SERVER
        else if (pfds[1].revents & POLLIN) {

            std::cout << std::endl;

            auto pair = Communication::readSocket(connfd);
            isServerSocketOk = pair.first;
            auto messages = pair.second;

            // RECEIVING MESSAGE FROM SOCKET AND INTERPRETING IT
            for (std::string& message : messages) {
                if (message == "")
                    continue;

                auto toServerMessages = Parser::parseServerMessage(message, client);

                if (client.isTimeToExit()) {
                    if (client.getStatus() == ClientStatus::ON_MATCH)
                        closingGame(pfds, &nfds, &clientConnFd, client);
                    close(connfd);
                    exit(0);
                }

                // AFTER SENDING A "BEGINRESP", CAN RECEIVE AN ACKNOWLEDGE BACK IF:
                // 1. THE RESPONSE FAILED TO (PLAYER OFFLINE, OR ON ANOTHER MATCH)
                // 2. THE RESPONSE WAS AN "ACCEPT", IN WHICH CASE THIS ACK WILL CONTAIN
                //      INFO FOR STARTING THE CONNECTION

                // In the case where the other player received the response but it was a rejection
                //  there's no reason for the client that sent that rejection to get a response for that.
                // On the other hand, if this response was an "accept", the acknoledge received here is more
                //  of a way for this client to get the necessary info to connect to the OTHER player that should
                //  a socket waiting for THIS client's connection
                TokenMatrix inTokenMatrix(message);
                if (inTokenMatrix[0][0] == "BEGINRESPACK") {
                    int i = inTokenMatrix.findLineIndex("STATUS");
                    if (inTokenMatrix[i][1] == "ok") {
                        int j = inTokenMatrix.findLineIndex("IP");
                        std::string ip = inTokenMatrix[j][1];

                        j = inTokenMatrix.findLineIndex("PORT");
                        std::string port = inTokenMatrix[j][1];

                        clientConnFd = connectToServer(ip.c_str(), std::stoi(port));

                        nfds = 3;
                        pfds[2].fd = clientConnFd;
                        pfds[2].events = POLLIN;

                        // Need to let server know that THIS client (the one who connected to the listening socket)
                        //  has begun
                        Communication::writeToSocket(connfd, "START " + client.getRivalUsername());
                    }
                }

                // AFTER RECEIVING MESSAGE FROM SOCKET, if there's some other message to be sent
                for (auto toServerMessage: toServerMessages) {
                    if (toServerMessage == "")
                        continue;

                    // AFTER RECEIVING A "BEGINRESP", SEND A "BEGINRESPACK" IN CASE OF
                    // FAILURE OR FOR STARTING THE GAME

                    // The "BEGINRESPACK" that's going to be send back to the client which
                    //  ACCEPTED OR NOT the invite sent by THIS client will contain, besides the
                    //  main information that came from THAT client, aditional information about where to
                    //  find THIS client so that they can start the match (IP ADDRESS AND PORT that THIS
                    //  client will be listining to) will be delivered back to THAT client (the PORT will come from
                    //  the code below, while the IP will be attached on the SERVER, during the route of this message)

                    TokenMatrix outTokenMatrix(toServerMessage);
                    if (outTokenMatrix[0][0] == "BEGINRESPACK") {
                        int i = outTokenMatrix.findLineIndex("STATUS");
                        if (outTokenMatrix[i][1] == "ok") {

                            // NOTE: THIS SOCKET IS NON-BLOCKING
                            int gameSocketListener = createGameListingSocket();
                            nfds = 3;
                            pfds[2].fd = gameSocketListener;
                            pfds[2].events = POLLIN;

                            struct sockaddr_in sin;
                            socklen_t len = sizeof(sin);
                            if (getsockname(gameSocketListener, (struct sockaddr *)&sin, &len) == -1) {
                                perror("getsockname");
                                exit(7);
                            } else {
                                std::string externalIP = getExternalIPAddress();
                                outTokenMatrix.addText("PORT " + std::to_string(ntohs(sin.sin_port)));
                                outTokenMatrix.addText("IP " + externalIP);
                            }

                            toServerMessage = outTokenMatrix.str();
                            isAcceptingConn = true;
                        }
                    }

                    Communication::writeToSocket(connfd, toServerMessage);
                }
            }
        }

        // This is run by the client how is listening for connections, and entering here means someone
        //  (presumably the client who is THIS client's adversary) has tried to connect ot the listening socket
        else if (isAcceptingConn && nfds == 3 && pfds[2].revents & POLLIN) {
            std::cout  << std::endl;

            isAcceptingConn = false;
            if ((clientConnFd = accept(pfds[2].fd, NULL, NULL)) == -1 ) {
                perror("Could not connect to other client!");
                client.setStatus(ClientStatus::LOGGED);
                client.setRivalUsername("");
                continue;
            }

            pfds[2].fd = clientConnFd;

            // Need to let server know that THIS client (the one who started listing first) has begun
            //  a match
            Communication::writeToSocket(connfd, "START " + client.getRivalUsername());
            if (client.isOnTurn()) {
                std::cout << "It's your turn!" << std::endl;
            } else {
                std::cout << "It's " + client.getRivalUsername() + "'s turn!" << std::endl;
            }
        }

        else if (nfds == 3 && pfds[2].revents & POLLIN) {
            std::cout << std::endl;

            auto pair = Communication::readSocket(clientConnFd);
            for (auto& message : pair.second) {

                auto resultingMessages = Parser::parseClientMessage(message, client);
                for (auto& resultingMessage : resultingMessages) {
                    if (resultingMessage == "")
                        continue;

                    // If play received from other player ended the game, notify server
                    TokenMatrix outTokenMatrix(resultingMessage);
                    if (outTokenMatrix[0][0] == "FINISH") {
                        Communication::writeToSocket(connfd, resultingMessage);
                        closingGame(pfds, &nfds, &clientConnFd, client);
                    }
                }
            }

            // No message "FINISH was sent" and other side of socket was closed = Connection lost
            if (nfds == 3 && !pair.first) {
                closingGame(pfds, &nfds, &clientConnFd, client);
                Communication::writeToSocket(connfd, "FINISH fail");
            }
        }

        std::cout << "JogoDaVelha> " << std::flush;
    }

    close(connfd);
    return 0;

}

#include <iostream>
#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>

#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "Player.hpp"
#include "Communication.hpp"
#include "Tokenizer.hpp"

#include "mainThread.hpp"
#include "clientThread.hpp"

// Write side of main thread pipe that receives messages from all threads
int mainPipeWriter;
std::mutex pipeWriterMutex;

// While user is not logged in, the only way for the main thread to know where to reply its message
//  is by association the thread's id to a write side of a pipe. When user logs in, this information will
//  be available inside the Player's object
std::unordered_map<std::string, int> threadIdToPipe;
std::mutex threadIdToPipeMutex;

std::unordered_map<std::string, Player> allPlayers;


// Get a socket listening on 'port_num'
int getListeningSocket(int port_num) {
    int listenfd;
    struct sockaddr_in servaddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port_num);
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    if (listen(listenfd, 1) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    return listenfd;
}

int main(int argc, char * argv[]) {

    if (argc != 2) {
        fprintf(stderr,"Use: %s <Port>\n",argv[0]);
        fprintf(stderr,"This is going to run a server on a TCP port <PORT>\n");
        exit(1);
    }

    int listenfd = getListeningSocket(atoi(argv[1]));

    // Generate main thread's pipe and start it
    int fd[2];
    pipe(fd);
    mainPipeWriter = fd[1];
    std::thread{mainThread, fd[0]}.detach();

    printf("[Server is live. Waiting for connections on port %s]\n",argv[1]);
    printf("[For termination, press CTRL+C]\n");

    while (1) {
        int connfd;

        struct sockaddr_in clientAddr;
        bzero(&clientAddr, sizeof(clientAddr));
        socklen_t socketLen;

        if ((connfd = accept(listenfd, (struct sockaddr *) &clientAddr, &socketLen)) == -1 ) {
            perror("Error during connection acceptance :(\n");
            exit(5);
        }

        // Get connecting client's IP address
        struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&clientAddr;
        struct in_addr ipAddr = pV4Addr->sin_addr;
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);

        std::thread{threadTask, connfd, std::string(str)}.detach();
    }

}
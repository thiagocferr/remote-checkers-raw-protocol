#ifndef COMMUNICATION_HPP
#define COMMUNICATION_HPP

#include <string>
#include <vector>

namespace Communication {

    // Given a socket FD, read its content and parse its input into a vector of strings.
    // If connection has been severed from the other side, return 'true' and a vector with the messages
    //  that could be gathered.
    // Else, return 'false' and a vector with all messages gathered
    std::pair<bool, std::vector<std::string>> readSocket(int connfd);

    // Write 'message' to socket of FD 'connfd'. Return 'false' if socket has been closed on the otherside,
    //  'true' if not
    bool writeToSocket(int connfd, std::string message);

    // Given a pipe FD, read its content and parse its input into a vector of strings.
    // If the pipe writing side has been closed, return 'true' and a vector with the messages
    //  that could be gathered.
    // Else, return 'false' and a vector with all messages gathered
    std::pair<bool, std::vector<std::string>> readPipe(int pipeReader);

    // Write 'message' to pipe of writer side FD 'pipeWriter'. Return 'false' if pipe has been closed on the otherside,
    //  'true' if not
    bool writeToPipe(int pipeWriter, std::string message);
}

#endif
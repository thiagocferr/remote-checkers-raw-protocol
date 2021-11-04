
#include "Communication.hpp"

#include <iostream>
#include <algorithm>
#include <utility>

#include <unistd.h>
#include <errno.h>

namespace Communication {

    namespace {

        std::pair<bool,std::vector<std::string>> commonRead(int fd) {

        // int n;
        // std::vector<char> buffer(1024);

        // n = read(mainPipeReader, &buffer[0], 1024);
        // std::string message(buffer.begin(), buffer.begin() + n);

        bool isFdOk;
        unsigned int bytesRead = 0;
        unsigned int currMessageIndex = 0;
        int n;

        std::vector<std::string> messages;
        std::vector<char> buffer(1024);

        while (true) {

            if (bytesRead == buffer.size()) {
                buffer.resize(buffer.size() * 2);
            }

            if ((n = read(fd, &buffer[bytesRead], buffer.size() - bytesRead)) < 0) {
                std::cerr << "Failed to read data from file descriptor.\n";
            } else if (n == 0) {
                isFdOk = false;
                break;
            }

            bytesRead += n;

            std::string message = std::string(buffer.begin(), buffer.begin() + bytesRead);
            std::string::size_type loc = message.find( "\n\n", currMessageIndex);

            while (loc != std::string::npos) {

                std::string actualMessage = std::string(buffer.begin() + currMessageIndex, buffer.begin() + loc);
                messages.push_back(std::move(actualMessage));

                currMessageIndex = loc + 2;
                loc = message.find("\n\n", currMessageIndex);
            }

            // Some message was not totally received so read again
            if (currMessageIndex == message.size()) {
                isFdOk = true;
                break;
            }

        }

        return std::pair<bool, std::vector<std::string>>(isFdOk, messages);
    }

        bool commonWrite(int fd, std::string message) {
            // Message delimiter. When communication through the network, this simbolizes the end of
            // the message
            message += "\n\n";

            std::vector<char> data(message.begin(), message.end());
            unsigned long messageSize = message.size();
            unsigned long bytesWritten = 0;

            bool isFdOk = true; //
            int n;

            while (bytesWritten < messageSize) {

                if ((n = write(fd, &data[0], messageSize - bytesWritten)) < 0) {
                    if (errno == EPIPE || errno == EIO) {
                        isFdOk = false;
                        break;
                    } else if (errno == EINTR) {
                        continue;
                    } else {
                        std::cerr << "Failed to write data to file descriptor" << std::endl;
                        isFdOk = false;
                        break;
                    }
                }

                bytesWritten += n;
            }
            return isFdOk;
        }
    }



    std::pair<bool,std::vector<std::string>> readSocket(int connfd) {
        return commonRead(connfd);
    }

    bool writeToSocket(int connfd, std::string message) {
        return commonWrite(connfd, std::move(message));
    }

    std::pair<bool, std::vector<std::string>> readPipe(int pipeReader) {
        return commonRead(pipeReader);
    }

    bool writeToPipe(int pipeWriter, std::string message) {
        return commonWrite(pipeWriter, std::move(message));
    }

}
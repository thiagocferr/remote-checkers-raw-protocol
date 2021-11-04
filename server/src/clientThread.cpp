
#include "clientThread.hpp"

#include <iostream>
#include <queue>
#include <unordered_map>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include "Player.hpp"
#include "Communication.hpp"
#include "Tokenizer.hpp"

void threadTask(int connfd, std::string ipAddress) {

    // Get threadID as string
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    std::string threadId = ss.str();

    int threadPipeFd[2];
    pipe(threadPipeFd);
    int threadPipeWriter = threadPipeFd[1];

    // Associate current thread with a place for writing messages
    threadIdToPipeMutex.lock();
        threadIdToPipe[threadId] = threadPipeWriter;
    threadIdToPipeMutex.unlock();

    // The socket or pipe connection has been severed
    bool isSocketEnd = false;
    bool isPipeEnd = false;

    // Polling preparation
    int threadPipeReader = threadPipeFd[0];
    struct pollfd *pfds;
    if ((pfds = (struct pollfd *) calloc(2, sizeof(struct pollfd))) == NULL) {
        perror("Malloc array of poll file descriptors");
        exit(6);
    }
    pfds[0].fd = connfd;
    pfds[0].events = POLLIN;
    pfds[1].fd = threadPipeReader;
    pfds[1].events = POLLIN;

    // If user is already logged, save its current username on server-side
    std::string loggedUser = "";
    std::string clientIP = ipAddress;

    // Just once, notify main thread that a new client has connected (CONNIN = Connection In)
    pipeWriterMutex.lock();
        Communication::writeToPipe(mainPipeWriter, "CONNIN " + clientIP);
    pipeWriterMutex.unlock();

    while (true) {

        if (poll(pfds, 2, -1) == -1) {
            std::cout << "THREAD FATAL ERROR: polling failed";
            break;
        }

        // Treat both cases outside infinite loop
        if (isSocketEnd || isPipeEnd) {
            free(pfds);

            pipeWriterMutex.lock();
                if (loggedUser != "")
                    Communication::writeToPipe(mainPipeWriter, "LOGOUT\nUSERNAME " + loggedUser);
                Communication::writeToPipe(mainPipeWriter, "TERMINATED\nIP " + clientIP);
            pipeWriterMutex.unlock();
            break;
        }

        // There's message on PIPE. For avoiding stopping the main thread, prioritize PIPE messages
        if (pfds[1].revents & POLLIN) {
            bool receivedExit = false;

            auto messages = Communication::readPipe(threadPipeReader);

            for (std::string message : messages.second) {

                TokenMatrix inTokenMatrix(message);
                if (inTokenMatrix[0][0] == "LOGINACK") {
                    loggedUser = inTokenMatrix[0][1];
                } else if (inTokenMatrix[0][0] == "LOGOUTACK") {
                    loggedUser = "";
                } else if (inTokenMatrix[0][0] == "LEADERS") {
                    // Order leaderboad before sending to client
                    std::string orderedMessage = "LEADERS ";
                    std::multimap<int, std::string, std::greater<int>> multimap;

                    for (unsigned int i = 1; i < inTokenMatrix[0].size(); i+=2) {
                        multimap.insert({std::stoi(inTokenMatrix[0][i + 1]), inTokenMatrix[0][i]});
                    }
                    for (auto it = multimap.begin(); it != multimap.end(); ++it) {
                        orderedMessage += it->second + " " + std::to_string(it->first) + " ";
                    }
                    orderedMessage.pop_back();
                    message = orderedMessage;
                } else if (inTokenMatrix[0][0] == "EXITACK") {
                    receivedExit = true;
                }

                Communication::writeToSocket(connfd, std::move(message));
            }

            if (receivedExit) {
                break;
            }

            // Write side of thread pipe has been close
            if (!messages.first)
                isPipeEnd = true;
        }

        // There's message on SOCKET
        else if (pfds[0].revents & POLLIN) {

            // Pass received messages on SOCKET to MAIN PIPE
            auto messages = Communication::readSocket(connfd);
            for (std::string message : messages.second) {

                TokenMatrix tokenMatrix(std::move(message));

                // For operations that require acess to the Player info, pass the name of the logged player
                if (tokenMatrix.findLineIndex("USERNAME") == -1)
                    if (loggedUser != "")
                        tokenMatrix.addText("USERNAME " + loggedUser);

                if (tokenMatrix[0][0] == "LOGIN") {
                    tokenMatrix.addText("IP " + ipAddress);
                }

                if (tokenMatrix[0][0] == "EXIT") {
                    tokenMatrix.addText("IP " + ipAddress);
                }

                // If the ACK from a match begin RESPONSE is coming from a client, this means that a match
                //  may be started right after, so we need to pass where THAT client can find THIS client.
                //  A PORT was already provided by the THIS client itself, but here is a better place for
                //  getting an IP address of THIS client
                if (tokenMatrix[0][0] == "BEGINRESPACK") {
                    int i = tokenMatrix.findLineIndex("IP");
                    if (i == -1)
                        tokenMatrix.addText("IP " + ipAddress);
                }

                // This will be used to know in which thread pipe to write responses back to
                tokenMatrix.addText("THREAD_ID " + threadId);

                pipeWriterMutex.lock();
                    Communication::writeToPipe(mainPipeWriter, tokenMatrix.str());
                pipeWriterMutex.unlock();
            }

            // Write side of socket (on client) has been closed
            if (!messages.first)
                isSocketEnd = true;
        }
    }

    threadIdToPipeMutex.lock();
        threadIdToPipe.erase(threadId);
    threadIdToPipeMutex.unlock();

    close(threadPipeWriter);
    close(threadPipeReader);
    close(connfd);
}
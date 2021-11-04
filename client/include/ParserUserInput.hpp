#ifndef PARSERUSERINPUT_H
#define PARSERUSERINPUT_H

#include <string>
#include <vector>

#include "Client.hpp"

namespace Parser {
    // For a user input / server message, parse and do business logic on Client
    std::vector<std::string> parseUserInput(const std::string& message, Client& client);
}

#endif
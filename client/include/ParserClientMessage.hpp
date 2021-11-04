#ifndef PARSERCLIENTMESSAGE
#define PARSERCLIENTMESSAGE

#include <string>
#include <vector>

#include "Client.hpp"

namespace Parser {
    std::vector<std::string> parseClientMessage(const std::string& message, Client& client);
}

#endif
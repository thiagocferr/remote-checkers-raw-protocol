#ifndef PARSERSERVERMESSAGE_HPP
#define PARSERSERVERMESSAGE_HPP

#include <string>
#include <vector>

#include "Client.hpp"

namespace Parser {

    std::vector<std::string> parseServerMessage(const std::string& message, Client& client);
}

#endif
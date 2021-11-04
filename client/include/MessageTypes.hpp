#ifndef MESSAGE_TYPES_HPP
#define MESSAGE_TYPES_HPP

#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <any>

#include "Client.hpp"

namespace MessageTypes {

    // NOTE: The insertion of struct 'MessageTypeInfo' on both maps below is done by the order of
    //  the declarations inside struct because this is a C++ feature only available at C++20, which because
    // of its recentness, we do not force the use of it

    // From a string, get a subtring with the message type (for now, all message types are
    // defined as the first word present on the text)
    std::string getMessageTypeSubstring(const std::string& text);

    // Get in which Client Statuses a certain command can be specified
    std::set<ClientStatus> getAllowedStatesForCommand(const std::string& userInput);
}

#endif
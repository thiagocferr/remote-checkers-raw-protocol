#include "MessageTypes.hpp"



namespace MessageTypes {

    namespace {

        std::unordered_map<std::string, std::set<ClientStatus>> userInputTypeMapping {
            {"adduser", {ClientStatus::NOT_LOGGED}},
            {"passwd", {ClientStatus::LOGGED}},
            {"login", {ClientStatus::NOT_LOGGED}},
            {"logout", {ClientStatus::LOGGED}},
            {"leaders", {ClientStatus::LOGGED, ClientStatus::WAITING_MATCH}},
            {"list", {ClientStatus::LOGGED, ClientStatus::WAITING_MATCH}},
            {"begin", {ClientStatus::LOGGED}},
            {"cancel", {ClientStatus::WAITING_MATCH}},
            {"invite", {ClientStatus::LOGGED}},
            {"send", {ClientStatus::ON_MATCH}},
            {"delay", {ClientStatus::ON_MATCH}},
            {"end", {ClientStatus::ON_MATCH}},
            {"exit", {ClientStatus::NOT_LOGGED, ClientStatus::LOGGED}},
        };

        std::set<ClientStatus> getAllowedStates(const std::string& msgType,
            std::unordered_map<std::string, std::set<ClientStatus>>& map) {

            auto search = map.find(msgType);
            if (search == map.end())
                return std::set<ClientStatus>();
            return search->second;
        }
    }

    std::string getMessageTypeSubstring(const std::string& text) {
        return text.substr(0, text.find(' '));
    }

    std::set<ClientStatus> getAllowedStatesForCommand(const std::string& userInput) {
        std::string command = getMessageTypeSubstring(userInput);
        return getAllowedStates(command, userInputTypeMapping);
    }

}






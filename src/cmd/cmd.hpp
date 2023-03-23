#pragma once
#include <string>
#include <unordered_map>
#include <functional>

class CmdExecuter {
public:
    void register_cmd(std::string command, std::function<void()> func) {
        commands_.emplace(std::move(command) , std::move(func));
    }

    void execute(const std::string& command) {
        auto it = commands_.find(command);
        if (it != commands_.end()) {
            it->second();
        }
    }

    bool contain(const std::string& command) {
        return commands_.count(command);
    }

private:
    std::unordered_map<std::string, std::function<void()>> commands_;
};

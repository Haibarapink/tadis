#pragma once

#include <functional>
#include <map>
#include <string>

class Command {
public:
  using Func = std::function<void()>;
  Command() = default;
  ~Command() = default;

  void add(std::string_view name, Func func)
  {
    commands_.emplace(name, func);
  }

  void run(std::string_view name)
  {
    if (commands_.find(name) != commands_.end()) {
      commands_[name]();
    }
  }

private:
    std::map<std::string_view, Func> commands_;
};


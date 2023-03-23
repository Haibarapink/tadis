#include "catalog/catalog.hpp"
#include "cmd/cmd.hpp"
#include "common/rc.hpp"
#include "result/result_stage.hpp"
#include "sql/parser/parer_stage.hpp"
#include "execution/exec_stage.hpp"
#include "cmd/cmd.hpp"

#include <cassert>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

bool quit = false;

// commands=====================================================================================================================================
// ".clear"
void clear_terminal() {
  std::cout << "\033[2J\033[1;1H";
}

// ".quit"
void do_quit() {
  quit = true;
}

// ".exec [files]"
void read_file_and_print(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return;
  }
  std::string line;
  while (std::getline(file, line)) {
    std::cout << line << std::endl;
  }
  file.close();
}

// ".license"
void print_license() {

  std::cout << "The MIT License (MIT)\n"
            << "\n"
            << "Copyright (c) 2021 "
            << "The MIT License (MIT)\n"
            << "\n"
            << "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n"
            << "\n"
            << "The above copyright notice and this permission notice shall be included in all "
            << "copies or substantial portions of the Software.\n"
            << "\n"
            << "The Software is provided “as is”, without warranty of any kind, express or implied, "
            << "including but not limited to the warranties of merchantability, fitness for a particular "
            << "purpose and noninfringement. In no event shall the authors or copyright holders be liable "
            << "for any claim, damages or other liability, whether in an action of contract, tort or "
            << "otherwise, arising from, out of or in connection with the software or the use or other "
            << "dealings in the Software." << std::endl;
}


CmdExecuter cmd;
// commands======================================================================================================

// register function
void init()
{
  cmd.register_cmd(".clear", clear_terminal);
  cmd.register_cmd(".quit", do_quit);
  cmd.register_cmd(".license", print_license);
}

std::string result2str(std::vector<Tuple> &ts, RC rc)
{
  std::string result = "";
  if (ts.empty()) {
    result += "Empty set\n";
    return result;
  }
  std::vector<size_t> max_lens(ts[0].size(), 0);
  for (auto &t : ts) {
    for (size_t i = 0; i < t.size(); i++) {
      max_lens[i] = std::max(max_lens[i], t[i].size());
    }
  }
  for (size_t i = 0; i < ts[0].size(); i++) {
    result += "+";
    for (size_t j = 0; j < max_lens[i] + 2; j++) {
      result += "-";
    }
  }
  result += "+\n";
  for (auto &t : ts) {
    result += "| ";
    for (size_t i = 0; i < t.size(); i++) {
      result += t[i].to_string();
      for (size_t j = 0; j < max_lens[i] - t[i].size() + 1; j++) {
        result += " ";
      }
      result += "| ";
    }
    result += "\n";
  }
  for (size_t i = 0; i < ts[0].size(); i++) {
    result += "+";
    for (size_t j = 0; j < max_lens[i] + 2; j++) {
      result += "-";
    }
  }
  result += "+\n";
  return result;
}

void result_cb2(std::vector<Tuple> &ts, RC rc) {
  auto str = result2str(ts, rc);
  std::cout << str;
}

void result_cb(std::vector<Tuple> &ts, RC rc)
{
  if (ts.empty()) {
    std::cout << "Empty set" << std::endl;
    return;
  }
  std::vector<size_t> max_lens(ts[0].size(), 0);
  for (auto &t : ts) {
    for (size_t i = 0; i < t.size(); i++) {
      max_lens[i] = std::max(max_lens[i], t[i].size());
    }
  }
  for (size_t i = 0; i < ts[0].size(); i++) {
    std::cout << "+";
    for (size_t j = 0; j < max_lens[i] + 2; j++) {
      std::cout << "-";
    }
  }
  std::cout << "+" << std::endl;
  for (auto &t : ts) {
    std::cout << "| ";
    for (size_t i = 0; i < t.size(); i++) {
      std::cout << t[i].to_string();
      for (size_t j = 0; j < max_lens[i] - t[i].size() + 1; j++) {
        std::cout << " ";
      }
      std::cout << "| ";
    }
    std::cout << std::endl;
  }
  for (size_t i = 0; i < ts[0].size(); i++) {
    std::cout << "+";
    for (size_t j = 0; j < max_lens[i] + 2; j++) {
      std::cout << "-";
    }
  }
  std::cout << "+" << std::endl;
}


void handle_error(const std::string &query, RC rc)
{
  std::cout << "\033[31m"
            << "Error: \n"
            << "        query: '" << query << "' " << rc2str(rc) << "\033[0m" << std::endl;
}

void exec_query(std::string query)
{
  std::string q = query;
  auto s = std::unique_ptr<Stage>(new ParsingStage{});
  ParsingStage *p = (ParsingStage *)s.get();
  p->init(std::move(query));
  RC rc = s->do_request();
  if (!rc_success(rc)) {
    handle_error(q, rc);
  }
}

void handle_cmd(const std::string &cmd_line)
{
  if (!cmd.contain(cmd_line)) {
    std::cout << "'" << cmd_line<< "'" << "is not existed!" << std::endl;
    return;
  }
  cmd.execute(cmd_line);
}

int main(int argc, char *argv[])
{
  if (!std::filesystem::exists(".tadis")) {
    std::filesystem::create_directory(".tadis");
  }

  print_license();
  init();

  Catalog::catalog().init(".tadis");
  ResultStage::defaultCb = result_cb;

  std::string q;
  while (!quit) {
    std::cout << "\033[32m"
              << "<Tadis 0.0.2>$ "
              << "\033[0m";
    std::getline(std::cin, q);
    if (q.size() > 0 && q[0] == '.')
      handle_cmd(q);
    else
      exec_query(std::move(q));
  }

  Catalog::catalog().table_manager()->close();
}
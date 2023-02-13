/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-08 13:35:35
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 05:02:54
 * @FilePath: /tadis/src/main.cc
 * @Description: a command line db
 */

#include "common/logger.hpp"
#include "common/rc.hpp"
#include "execution/execution.hpp"
#include "session/session.hpp"
#include "sql/parser/parser.hpp"
#include "storage/tuple.hpp"
#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/stat.h>

void usage()
{
  std::cout << "Usage : tadis [database directory]";
}

void print_error(RC rc)
{
  std::cout << "Tadis> "
            << "Error : " << rc2str(rc) << std::endl;
}

void print_success()
{
  std::cout << "Tadis> Success!" << std::endl;
}

void handle_query(std::string &query)
{
  Parser<std::string_view> parser{std::string_view{query.data(), query.size()}};
  RC rc = parser.parse();
  if (!rc_success(rc)) {
    print_error(rc);
    return;
  }
  Executer executer;
  executer.init(const_cast<QueryAst &>(parser.query()));
  auto res = executer.exec();
  if (!rc_success(res.rc)) {
    print_error(res.rc);
    return;
  }

  if (!res.tuples_.empty()) {
    auto tuple_meta = res.tuples_[0].tuple_meta();

    // print head
    for (auto &&tcm : tuple_meta->cells_) {
      std::cout << "| " << tcm.name_ << " |";
    }
    std::cout << "\n";

    for (auto &tuple : res.tuples_) {
      TupleCell tcl;
      for (auto i = 0; rc_success(tuple.get_cell(i, tcl)); i++) {
        std::cout << "| " << tcl.to_string() << " |";
      }
      std::cout << "\n";
    }

    std::cout << std::endl;
  }
}

int main(int argc, char *argv[])
{
  mkdir(".tadis", S_IRWXU);

  if (!std::filesystem::is_regular_file(std::filesystem::path{".tadis/tadis.log"})) {
    FILE *f = fopen(".tadis/tadis.log", "w+");
    fclose(f);
  }

  Logger::init_logger(".tadis/tadis.log");

  std::string dir{".tadis"};
  RC rc = init_gloabl_session(std::string_view{dir.data(), dir.size()});
  if (rc != RC::SUCCESS) {
    std::cout << "init gloal session fail";
    return 1;
  }

  std::string line;

  while (true) {
    std::cout << "Tadis >";
    std::getline(std::cin, line);
    if (line == "quit")
      break;
    else {
      handle_query(line);
    }

    line.clear();
  }

  auto tm = GlobalSession::global_session().table_manager();
  tm->close();
}
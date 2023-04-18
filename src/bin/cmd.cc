#include "binder/binder.hpp"
#include "catalog/catalog.hpp"
#include "sql/parser/parser.hpp"
#include <string>

Catalog catalog;

void handle_cmds(const std::string &cmd)
{}

int main(int argc, char *argv[])
{
  std::string cmd;
  while (std::getline(std::cin, cmd)) {}
}
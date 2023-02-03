/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-31 20:03:58
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-31 20:39:10
 * @FilePath: /tadis/src/example.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 */
#include "common/rc.hpp"
#include "sql/parser/ast.hpp"
#include "sql/parser/parser.hpp"
#include <iostream>
#include <variant>

void handle_create_table(const CreateTableAst &create_table)
{}

void handle_select(const SelectAst &select)
{}

void handle_delete(const DeleteAst &del)
{}

void handle_insert(const InsertAst &insert)
{}

void handle_sql(const std::string &line)
{
  Parser<std::string_view> parser{std::string_view{line.data(), line.size()}};
  if (auto rc = parser.parse(); !rc_success(rc)) {
    std::cout << "Tadis> FAIL : " << rc2str(rc) << std::endl;
  }
  auto &&query = parser.query();
  if (std::holds_alternative<SelectAst>(query)) {
    handle_select(std::get<SelectAst>(query));
  } else if (std::holds_alternative<CreateTableAst>(query)) {
    handle_create_table(std::get<CreateTableAst>(query));
  } else if (std::holds_alternative<DeleteAst>(query)) {
    handle_delete(std::get<DeleteAst>(query));
  } else if (std::holds_alternative<InsertAst>(query)) {
    handle_insert(std::get<InsertAst>(query));
  } else {
    std::cout << "Tadis> Error Query Type" << std::endl;
  }
}

int main(int, char *[])
{
  std::string line;
  while (true) {
    std::cin >> line;
    if (line == "quit") {
      break;
    } else {
      handle_sql(line);
    }
  }
}
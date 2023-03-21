/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-03-11 13:32:36
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-03-11 13:32:41
 * @FilePath: /tadis/src/statement/create_table_stmt.hpp
 * @Description: Create table stmt
 */
#pragma once

#include "sql/parser/ast.hpp"

#include <string>

class CreateTbaleStmt {
public:
  std::string table_name;
  std::vector<ColAttr> cols_;
};
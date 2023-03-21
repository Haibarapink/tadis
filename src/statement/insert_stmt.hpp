/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-03-11 11:03:33
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-03-11 11:03:55
 * @FilePath: /tadis/src/statement/insert_stmt.hpp
 * @Description: Statement
 */
#pragma once

#include "storage/table.hpp"

class InsertStmt {
public:
  Table *table_;
  std::vector<Value> values_;
};
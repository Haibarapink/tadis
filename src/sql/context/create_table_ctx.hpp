/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:01:58
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-15 11:24:29
 * @FilePath: /tadis/src/sql/context/create_table_ctx.hpp
 */
#pragma once

#include "sql/parser/ast.hpp"
#include "storage/tuple.hpp"

class CreateTableCtx {
public:
  // nothing
  std::string name_;
  TupleMeta table_meta_;
  std::string base_dir_;
};
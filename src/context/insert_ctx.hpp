/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:01:27
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 01:54:54
 * @FilePath: /tadis/src/sql/context/insert_ctx.hpp
 * @Description: 插入
 */
#pragma once

#include "session/session.hpp"
#include "sql/parser/ast.hpp"

class InsertContext {
public:
  RC init(InsertAst &ast)
  {
    query_ = &ast;
    auto &&tm = GlobalSession::global_session().table_manager();
    if (!tm->contain(ast.table_name_)) {
      return RC::TABLE_NOT_EXISTED;
    }
    table_ = tm->table(ast.table_name_);
    return RC::SUCCESS;
  }

  RC exec()
  {
    assert(query_ && table_);
    return table_->insert(query_->cols_, query_->values_);
  }

private:
  InsertAst *query_ = nullptr;
  Table *table_ = nullptr;
};
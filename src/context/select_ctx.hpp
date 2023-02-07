/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:01:41
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 03:22:51
 * @FilePath: /tadis/src/sql/context/select_ctx.hpp
 * @Description:
 */
#pragma once

#include "sql/parser/ast.hpp"
#include "session/session.hpp"
#include "execution/result.hpp"
#include "storage/table.hpp"
#include "storage/tuple.hpp"

#include <map>

class SelectContext {
public:
  RC init(SelectAst &ast);
  RC exec(QueryResult &res);

private:
  SelectAst *query_ = nullptr;
  Table *table_ = nullptr;
};

inline RC SelectContext::init(SelectAst &ast)
{
  auto tm = GlobalSession::global_session().table_manager();

  if (ast.from_list_.empty() || ast.selist_.empty()) {
    return RC::INTERNAL_ERROR;
  }

  if (!tm->contain(ast.from_list_[0])) {
    return RC::TABLE_NOT_EXISTED;
  }

  table_ = tm->table(ast.from_list_[0]);
  query_ = &ast;

  return RC::SUCCESS;
}

// 暂时全部输出
inline RC SelectContext::exec(QueryResult &res)
{
  res.tuples_ = table_->tuples();
  res.rc = RC::SUCCESS;
  return RC::SUCCESS;
}
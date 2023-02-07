/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 12:54:19
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-07 18:54:00
 * @FilePath: /tadis/src/sql/execution/execution.hpp
 * @Description: 执行层, 解析完毕后传入Operator，在此执行
 */
#pragma once
#include "execution/result.hpp"
#include "sql/parser/ast.hpp"
#include "context/create_table_ctx.hpp"
#include <algorithm>
#include <cassert>
#include <variant>

class Executer {
public:
  void init(QueryAst &q)
  {
    query_ = &q;
  }

  QueryResult exec();

private:
  void handle_create_table(CreateTableAst &ast, QueryResult &res);

  QueryAst *query_ = nullptr;
};

inline QueryResult Executer::exec()
{
  assert(query_);
  QueryResult res;

  if (std::holds_alternative<SelectAst>(*query_)) {
  } else if (std::holds_alternative<DeleteAst>(*query_)) {

  } else if (std::holds_alternative<CreateTableAst>(*query_)) {
    handle_create_table(std::get<CreateTableAst>(*query_), res);
  } else {
  }

  return res;
}

inline void Executer::handle_create_table(CreateTableAst &ast, QueryResult &res)
{
  CreateTableCtx ctx;
  ctx.init(ast);
  RC rc = ctx.exec();
  res.rc = rc;
}

/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 12:54:19
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 03:08:42
 * @FilePath: /tadis/src/sql/execution/execution.hpp
 * @Description: 执行层, 解析完毕后传入Operator，在此执行
 */
#pragma once
#include "common/rc.hpp"
#include "execution/result.hpp"
#include "sql/parser/ast.hpp"
#include "context/insert_ctx.hpp"
#include "context/select_ctx.hpp"
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
  template <typename T, typename V>
  bool is(V &v)
  {
    return std::holds_alternative<T>(v);
  }

  void handle_create_table(CreateTableAst &ast, QueryResult &res);
  void handle_select_record(SelectAst &ast, QueryResult &res);
  void handle_delete_record(DeleteAst &ast, QueryResult &res);
  void handle_insert_record(InsertAst &ast, QueryResult &res);

  QueryAst *query_ = nullptr;
};

inline QueryResult Executer::exec()
{
  assert(query_);
  QueryResult res;

  if (is<SelectAst>(*query_)) {
    handle_select_record(std::get<SelectAst>(*query_), res);
  } else if (is<DeleteAst>(*query_)) {
    handle_delete_record(std::get<DeleteAst>(*query_), res);
  } else if (is<CreateTableAst>(*query_)) {
    handle_create_table(std::get<CreateTableAst>(*query_), res);
  } else if (is<InsertAst>(*query_)) {
    handle_insert_record(std::get<InsertAst>(*query_), res);
  }

  return res;
}

inline void Executer::handle_create_table(CreateTableAst &ast, QueryResult &res)
{
  CreateTableCtx ctx;
  auto rc = ctx.init(ast);
  if (!rc_success(rc)) {
    res.rc = rc;
    return;
  }
  rc = ctx.exec();
  res.rc = rc;
}

inline void Executer::handle_select_record(SelectAst &ast, QueryResult &res)
{
  SelectContext select_ctx;
  if (res.rc = select_ctx.init(ast); !rc_success(res.rc)) {
    return;
  }
  select_ctx.exec(res);
}

inline void Executer::handle_delete_record(DeleteAst &ast, QueryResult &res)
{}

inline void Executer::handle_insert_record(InsertAst &ast, QueryResult &res)
{
  InsertContext ctx;
  res.rc = ctx.init(ast);

  if (!rc_success(res.rc)) {
    return;
  }

  res.rc = ctx.exec();
}

#pragma once

// Client Context, include catalog, transaction, etc.
#include "catalog/catalog.hpp"
#include "execution/result.hpp"
#include "sql/parser/ast.hpp"
#include "sql/stmt.hpp"

class ClientContext {
public:
  ClientContext(SqlStmt stmt, Catalog *catalog) : stmt_(std::move(stmt)), catalog_(catalog)
  {
    result_ = new QueryResult();
    if (stmt.is_create_table()) {
      result_->type_ = ResultType::CREATE_TABLE;
    } else if (stmt.is_drop()) {
      result_->type_ = ResultType::DROP_TABLE;
    } else if (stmt.is_insert()) {
      result_->type_ = ResultType::INSERT;
    } else if (stmt.is_select()) {
      result_->type_ = ResultType::SELECT;
    } else if (stmt.is_delete()) {
      result_->type_ = ResultType::DELETE;
    } else {
      assert(false && "uninitialized stmt");
    }
  }

  SqlStmt stmt_;
  QueryResult *result_ = nullptr;
  Catalog *catalog_ = nullptr;
};
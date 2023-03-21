#pragma once
#include "statement/insert_stmt.hpp"

#include "operator/operator.hpp"

class InsertOp : public Operator {
public:
  InsertOp(InsertStmt stmt) : stmt_(std::move(stmt))
  {}

  RC next(std::vector<Tuple> *tuples)
  {
    RC rc = stmt_.table_->insert(nullptr, stmt_.values_);
    return rc;
  }

  bool has_next()
  {
    return done_;
  }

  RC close()
  {
    return RC::SUCCESS;
  }

private:
  bool done_ = false;
  InsertStmt stmt_;
};
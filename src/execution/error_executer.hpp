#pragma once

#include "execution/execution.hpp"
#include "execution/result.hpp"

class ErrorExecuter : public Executer {
public:
  ErrorExecuter(RC rc, QueryResult *result) : rc_(rc)
  {}

  RC next(Tuple *) override
  {
    assert(!rc_success(rc_));
    return rc_;
  }

  RC rc_ = RC::INTERNAL_ERROR;
  QueryResult *query_result_;
};
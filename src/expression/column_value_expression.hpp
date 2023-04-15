#pragma once

#include "expression/expression.hpp"
#include "catalog/tuple.hpp"
#include "catalog/schema.hpp"

#include <cstddef>

class ColumnValueExpression : public Expression {
public:
  ColumnValueExpression(size_t idx) : idx_(idx)
  {}

  Value evaluate(Tuple *tuple, Schema *schema) override
  {
    return tuple->value_at(schema, idx_);
  }

private:
  size_t idx_ = 0;
};
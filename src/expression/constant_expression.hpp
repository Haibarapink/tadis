#pragma once

#include "catalog/tuple.hpp"
#include "expression/expression.hpp"
#include "sql/stmt.hpp"
#include "catalog/schema.hpp"

class ConstantExpression : public Expression {
public:
  ConstantExpression(Value val) : val_(val)
  {}

  Value evaluate(Tuple *, Schema *schema) override
  {
    return val_;
  }

  std::string to_string()
  {
    return val_.to_string();
  }

private:
  Value val_;
};
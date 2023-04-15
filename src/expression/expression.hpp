#pragma once

#include <memory>
#include <vector>

#include "catalog/schema.hpp"
#include "sql/stmt.hpp"
#include "catalog/tuple.hpp"

class Expression {
  using ExpressUniquePtr = std::unique_ptr<Expression>;

public:
  virtual ~Expression() = default;
  virtual Value evaluate(Tuple *, Schema *) = 0;
  // virtual Value evaluate_join(const Tuple &, const Tuple &) const = 0;
  std::vector<ExpressUniquePtr> children_;
};
#pragma once

#include <vector>
#include <memory>

#include "expression/expression.hpp"
#include "catalog/tuple.hpp"

class Executer {
public:
  virtual ~Executer() = default;

  virtual RC next(Tuple*) = 0;

  std::unique_ptr<Expression> expression_ = nullptr;
  std::unique_ptr<Schema> schema_ = nullptr;
  std::vector<std::unique_ptr<Executer>> children_;
};
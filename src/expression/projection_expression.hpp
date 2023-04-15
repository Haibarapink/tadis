#pragma once

#include "expression/expression.hpp"

class ProjectionExpression : public  Expression {
public:
  ProjectionExpression(std::vector<std::unique_ptr<Expression>> children) {
    children_ = std::move(children);
  }

  Value evaluate(Tuple *, Schema *) override {
    assert(false && "projection expression doesn't design for evaluate in my design...");
    return Value{};
  }
};
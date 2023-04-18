#pragma once

#include "expression/expression.hpp"

class FilterExpression : public Expression {
public:
  FilterExpression(std::vector<std::unique_ptr<Expression>> children)
  {
    children_ = std::move(children);
  }

  Value evaluate(Tuple *t, Schema *s) override
  {
    Value v;
    v = 1;
    for (auto &child : children_) {
      v = child->evaluate(t, s);
      if (v.type_ != AttrType::INTS) {
        assert(false);
      }

      if (v.get<long>() == -1) {
        return v;
      }
    }

    return v;
  }
};
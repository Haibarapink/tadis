#pragma once

#include "sql/parser/ast.hpp"
#include <ios>
#include <vector>

// left is value right is ...
class FilterStmt {
public:
  FilterStmt(std::vector<Condition> cnds)
  {
    for (auto &c : cnds) {
      if (c.left_.type_ == AttrType::REL_ATTR && c.right_.type_ != AttrType::REL_ATTR) {
        std::swap(c.left_, c.right_);

        if (c.op_ == CondOp::GREATER) {
          c.op_ = CondOp::SMALLER;
        } else if (c.op_ == CondOp::SMALLER) {
          c.op_ = CondOp::GREATER;
        }
      }
    }
    cnds_ = std::move(cnds);
  }

  std::vector<Condition> cnds_;
};
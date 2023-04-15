#pragma once

#include "catalog/schema.hpp"
#include "expression/expression.hpp"
#include "sql/parser/ast.hpp"

enum class ComparsionType : int {
  EQUAL,
  NOT_EQUAL,
  LESS_THAN,
  GREATER_THAN,
};
inline bool convert(CondOp op, ComparsionType& type) {
  switch (op) {
    case CondOp::GREATER:
      type = ComparsionType::GREATER_THAN;
      break;
    case CondOp::NOT_EQ:
      type = ComparsionType::NOT_EQUAL;
      break ;
    case CondOp::SMALLER:
      type = ComparsionType::LESS_THAN;
      break;
    case CondOp::EQ:
      type = ComparsionType::EQUAL;
      break ;
    default:
      return false;
  }
  return true;
}

// left expression is column_value_expression, and right expression is constant_expression
class ComparsionExpression : public Expression {
public:
  ComparsionExpression(Expression *left, Expression *right, ComparsionType type) : type_(type)
  {
    children_.push_back(std::unique_ptr<Expression>(left));
    children_.push_back(std::unique_ptr<Expression>(right));
  }

  // @brief return -1 or 1, -1 mean false, 1 mean true
  Value evaluate(Tuple *tuple, Schema *schema) override
  {
    Value left = children_[0]->evaluate(tuple, schema);   // fetch the column's value
    Value right = children_[1]->evaluate(tuple, schema);  // fetch the const value (from constant exression)
    Value res;
    int compare_res = left.compare(right);
    switch (type_) {
      case ComparsionType::EQUAL: {
        if (compare_res != 0) {
          // not eq
          res.init(-1);
        } else {
          res.init(1);
        }
      } break;
      case ComparsionType::NOT_EQUAL: {
        if (compare_res == 0) {
          // not eq
          res.init(-1);
        } else {
          res.init(1);
        }
      } break;
      case ComparsionType::LESS_THAN: {
        if (compare_res >= 0) {
          res.init(-1);
        } else {
          res.init(1);
        }
      } break;
      case ComparsionType::GREATER_THAN: {
        if (compare_res <= 0) {
          res.init(-1);
        } else {
          res.init(1);
        }
      } break;
      default:
        assert(false && "unimplemented");
    }
    return res;
  }

  ComparsionType type_;
};
#pragma once

#include "execution/execution.hpp"
#include "sql/parser/ast.hpp"

class FilterExecuter : public Executer {
public:
  FilterExecuter(Expression *expr, Executer *child)
  {
    expression_ = std::unique_ptr<Expression>{expr};
    children_.emplace_back(child);
  }

  RC next(Tuple *t) override
  {
    RC rc = RC::SUCCESS;
    for (;;) {
      rc = children_[0]->next(t);
      if (!rc_success(rc)) {
        return rc;
      }

      auto ok_value = expression_->evaluate(t, children_[0]->schema_.get());
      if (ok_value.get<long>() == 1) {
        break;
      }
    }

    return RC::SUCCESS;
  }
};
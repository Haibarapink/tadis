#pragma once

#include "execution/execution.hpp"

class ProjecetionExecuter : public Executer {
public:

  RC next(Tuple* tuple) override {
    std::vector<Value> values;
     Tuple child_tuple{{}};
    auto rc= children_[0]->next(&child_tuple);

    if (!rc_success(rc)) {
      return rc;
    }
    for (auto & expr : expression_->children_) {
      values.push_back(expr->evaluate(&child_tuple, children_[0]->schema_.get()));
    }
    *tuple = Tuple::create_tuple(values, schema_.get());
    return RC::SUCCESS;
  }


};
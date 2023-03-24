#pragma once

#include "catalog/catalog.hpp"
#include "operator/operator.hpp"

class DropOp : public Operator {
public:
  DropOp(std::string name) : table_(std::move(name))
  {}

  RC next(std::vector<Tuple> *ts)
  {
    done_ = true;
    TableManager *tm = Catalog::catalog().table_manager();
    return tm->drop_table(table_);
  }

  bool has_next() {
    return done_ == false;
  }

  RC close() {
    return RC::SUCCESS;
  }

private:
  bool done_ = false;
  std::string table_;
};

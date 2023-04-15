#pragma once

#include "execution/execution.hpp"
#include "catalog/catalog.hpp"

class CreateTableExecuter : public Executer {
public:
  CreateTableExecuter(std::string table_name, std::vector<Column> cols, Catalog* catalog) : name_(std::move(table_name)), cols_(cols), catalog_(catalog) {}

  RC next(Tuple *) override {
    assert(catalog_ || !cols_.empty());
    RC rc = RC::OUT_OF_RANGE;
    catalog_->create_table(name_, cols_);
    return rc;
  }

  std::string name_;
  std::vector<Column> cols_;
  Catalog* catalog_;
};
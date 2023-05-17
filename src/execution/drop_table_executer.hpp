#pragma once

#include "catalog/catalog.hpp"
#include "execution/execution.hpp"

class DropTableExecuter : public Executer {
public:
  DropTableExecuter(Catalog* catalog, std::string table_name) :catalog_(catalog), table_name_(table_name) {}

  RC next(Tuple *tuple) override {
    if (!catalog_->has_table(table_name_)) {
      return RC::TABLE_NOT_EXISTED;
    }
    catalog_->drop_table(table_name_);
    return RC::OUT_OF_RANGE;
  }

private:
  std::string table_name_;
  Catalog* catalog_ = nullptr;
};
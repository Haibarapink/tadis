#pragma once

#include "execution/execution.hpp"

class InsertExecuter : public Executer {
public:
  InsertExecuter(StorageManager* stm, Table* table, Schema* schema, std::vector<Value> vals) :
    stm_(stm), table_(table), schema_(schema), values_(std::move(vals)) {}

  RC next(Tuple *) override {
    assert(schema_ && stm_);
    RC rc = RC::OUT_OF_RANGE;
    Tuple t = Tuple::create_tuple(values_, schema_);
    auto heap = stm_->get_table(table_->name());
    assert(heap);
    heap->insert(Record{std::move(t.data_)});
    return rc;
  }

  StorageManager *stm_ = nullptr;
  Table *table_ = nullptr;
  Schema* schema_ = nullptr;
  std::vector<Value> values_;
};
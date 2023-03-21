#pragma once

#include "common/rc.hpp"
#include "operator/operator.hpp"
#include "storage/io/record.hpp"
#include "storage/table.hpp"
#include "storage/tuple.hpp"

#include <cassert>
#include <vector>

class TableScanOp : public Operator {
public:
  TableScanOp(Table *t) : table_(t)
  {
    scanner_.init(t->bfp_.get());
  }

  RC next(std::vector<Tuple> *tuples)
  {
    assert(tuples);
    while (scanner_.has_next()) {
      Record rec;
      RecordId rid;
      Tuple t;
      scanner_.next(rec, rid);
      rec.rid() = rid;
      t.init(&(table_->table_meta_.tuple_meta()), std::move(rec.data()));
      tuples->emplace_back(std::move(t));
    }
    return RC::SUCCESS;
  }

  bool has_next()
  {
    return scanner_.has_next();
  }

  RC close()
  {
    return RC::SUCCESS;
  }

private:
  Table *table_;
  RecordScanner scanner_;
};
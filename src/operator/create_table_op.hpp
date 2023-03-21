#pragma once

#include "catalog/catalog.hpp"
#include "common/rc.hpp"
#include "operator/operator.hpp"
#include "statement/create_table_stmt.hpp"
#include "storage/table.hpp"
#include "storage/tuple.hpp"

class CreateTableOp : public Operator {
public:
  CreateTableOp(CreateTbaleStmt stmt) : stmt_(std::move(stmt))
  {}

  RC next(std::vector<Tuple> *tuples)
  {
    done_ = true;
    auto tm = Catalog::catalog().table_manager();
    if (tm->contain(stmt_.table_name)) {
      return RC::TABLE_ALREADY_EXISTED;
    }

    TableMeta table_meta;
    std::vector<TupleCellMeta> tcm;
    for (auto &i : stmt_.cols_) {
      tcm.emplace_back(TupleCellMeta::init(i));
    }
    table_meta.init(stmt_.table_name, std::move(tcm));

    return tm->create_table(stmt_.table_name, std::move(table_meta));
  }

  bool has_next()
  {
    return done_ == false;
  }

  RC close()
  {
    return RC::SUCCESS;
  }

private:
  bool done_ = false;
  CreateTbaleStmt stmt_;
};
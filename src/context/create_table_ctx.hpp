/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:01:58
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-07 17:41:50
 * @FilePath: /tadis/src/sql/context/create_table_ctx.hpp
 */
#pragma once

#include "common/utility.hpp"
#include "sql/parser/ast.hpp"
#include "session/session.hpp"
#include "storage/table.hpp"
#include "storage/tuple.hpp"

#include <cassert>

class CreateTableCtx {
public:
  CreateTableCtx()
  {}

  RC init(CreateTableAst &ast)
  {
    auto &&session = GlobalSession::global_session();
    assert(session.table_manager());
    auto tm = session.table_manager();
    if (tm->contain(ast.table_name_)) {
      return RC::TABLE_ALREADY_EXISTED;
    }

    name_ = ast.table_name_;
    init_table_metas(ast.col_attrs_);

    return RC::SUCCESS;
  }

  RC exec()
  {
    auto &&session = GlobalSession::global_session();
    TableMeta table_meta;
    table_meta.init(std::string_view{name_.data(), name_.size()}, std::move(tuple_metas_.cells_));
    auto tm = session.table_manager();
    return tm->create_table(name_, std::move(table_meta));
  }

  void init_table_metas(std::vector<ColAttr> &attrs)
  {
    for (auto &&attr : attrs) {
      TupleCellMeta meta;
      to_upper(attr.type_);

      meta.name_ = std::move(attr.name_);

      if (attr.type_ == "VARCHAR") {
        meta.type_ = TupleCellType::VARCHAR;
        meta.len_ = attr.size_;
      } else if (attr.type_ == "CHAR") {
        meta.type_ = TupleCellType::CHAR;
        meta.len_ = attr.size_;
      } else if (attr.type_ == "FLOAT") {
        meta.type_ = TupleCellType::FLOAT;
        meta.len_ = 0;
      } else if (attr.type_ == "INT") {
        meta.type_ = TupleCellType::INTEGER;
        meta.len_ = 0;
      } else {
        assert(false);
      }

      tuple_metas_.cells_.emplace_back(std::move(meta));
    }
  }

  std::string name_;
  TupleMeta tuple_metas_;

  // table manager
};
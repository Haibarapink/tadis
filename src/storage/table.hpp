/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:03:29
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-07 15:33:43
 * @FilePath: /tadis/src/storage/table.hpp
 * @Description: Table
 */
#pragma once
#include "common/utility.hpp"
#include "sql/parser/ast.hpp"
#include "storage/io/buffer_pool.hpp"
#include "storage/kv/storage.hpp"
#include "storage/tuple.hpp"
#include <any>
#include <boost/asio/detail/descriptor_ops.hpp>
#include <boost/json/value.hpp>
#include <memory>
#include <string>
#include <boost/core/noncopyable.hpp>
#include <utility>

class TableMeta {
public:
  friend class Table;
  TableMeta() = default;
  TableMeta(const TableMeta &other)
  {
    copy(other);
  }

  TableMeta &operator=(const TableMeta &other)
  {
    copy(other);
    return *this;
  }

  TableMeta(TableMeta &&other)
  {
    move_from(std::move(other));
  }

  TableMeta &operator=(TableMeta &&other)
  {
    move_from(std::move(other));
    return *this;
  }

  void copy(const TableMeta &other)
  {
    this->meta_ = other.meta_;
    this->name_ = other.name_;
  }

  void move_from(TableMeta &&other)
  {
    this->meta_ = std::move(other.meta_);
    this->name_ = std::move(other.name_);
  }

  void init(std::string_view name, std::vector<TupleCellMeta> metas)
  {
    name_ = name;
    meta_.cells_ = std::move(metas);
  }

  RC cell_meta_at(size_t idx, TupleCellMeta &m)
  {
    if (idx >= meta_.cells_.size()) {
      return RC::OUT_OF_RANGE;
    }
    m = meta_.cells_[idx];
    return RC::SUCCESS;
  }
  RC cell_meta_at(std::string_view name, TupleCellMeta &m)
  {
    for (auto &&cell : meta_.cells_) {
      if (cell.name_ == name) {
        m = cell;
        return RC::SUCCESS;
      }
    }

    return RC::OUT_OF_RANGE;
  }

  RC from_json(const boost::json::value &v);
  boost::json::value to_json();

private:
  std::string name_;
  TupleMeta meta_;
};

class Table : public boost::noncopyable {
public:
  friend class TableManager;
  friend class TableTester;

  void init(std::string_view dir, TableMeta meta)
  {
    dir_path_ = dir;
    db_filename_ = make_data_filename(dir, meta.name_);
    bfp_ = std::unique_ptr<BufferPool>{new BufferPool{std::string_view{db_filename_.data(), db_filename_.size()}}};
  }

  // TODO
  RC insert(const std::vector<Value> &value)
  {}

  // TODO
  RC remove()
  {}

private:
  TableMeta table_meta_;
  std::string dir_path_;
  std::string db_filename_;
  std::unique_ptr<BufferPool> bfp_ = nullptr;
};

#include "storage/serilze/table.ipp"

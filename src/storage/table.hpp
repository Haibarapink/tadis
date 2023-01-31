/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:03:29
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-16 10:46:14
 * @FilePath: /tadis/src/storage/table.hpp
 * @Description: Table
 */
#pragma once
#include "common/utility.hpp"
#include "storage/kv/storage.hpp"
#include "storage/tuple.hpp"
#include <any>
#include <boost/json/value.hpp>
#include <memory>
#include <string>
#include <boost/core/noncopyable.hpp>

class TableMeta {
public:
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

template <typename StorageType>
class Table : public boost::noncopyable {
public:
  Table() = default;
  ~Table() = default;

  void init(std::string_view name, TableMeta meta, std::string_view dir)
  {
    name_ = name;
    table_meta_ = std::move(meta);
    dir_path_ = dir;
    data_file_name_.append(make_data_filename(dir, name));
  }

  void insert_record(BytesView rec);

private:
  std::string name_;
  std::string dir_path_;
  std::string data_file_name_;
  TableMeta table_meta_;
  StorageType store_;
};

#include "storage/serilze/table.ipp"

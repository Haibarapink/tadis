/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:03:29
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-13 18:40:22
 * @FilePath: /tadis/src/storage/table.hpp
 * @Description: Table
 */
#pragma once
#include "storage/kv/storage.hpp"
#include "storage/tuple.hpp"
#include <any>
#include <memory>
#include <string>
#include <boost/core/noncopyable.hpp>

class TableMeta : public boost::noncopyable {};

template <typename StorageType>
class Table : public boost::noncopyable {
public:
  Table() = default;
  ~Table() = default;

  void init(std::string_view name, TupleMeta meta, std::string_view dir)
  {
    name_ = name;
    tuple_meta_ = meta;
    dir_path_ = dir;
  }

  void insert_record(BytesView rec);

private:
  std::string name_;
  std::string dir_path_;
  TupleMeta tuple_meta_;
  StorageType store_;
};
/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-15 11:29:49
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-15 22:09:41
 * @FilePath: /tadis/src/storage/db.hpp
 * @Description: db 打开${base_dir}/db_${name}.json 用来管理 db 的元数据 比如保存的table的名字, index等等, table
 数据存放在
 ${base_dir}/table_${name}_data.db, 元信息存放在 ${base_dir}/table_${name}_meta.json 中
 */

#pragma once

#include "common/json.hpp"
#include "common/logger.hpp"
#include "common/rc.hpp"
#include "storage/io/fileop.hpp"
#include "storage/table.hpp"
#include "common/utility.hpp"
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

template <typename StorageType>
class Db {
public:
  Db(std::string_view base_dir, std::string_view db_meta) : base_dir_(base_dir), name_(db_meta)
  {
    init();
  }

  RC create_table(std::string_view name);

  void shutdown();

private:
  RC init();
  RC open_table(std::string_view table_name);

private:
  std::string base_dir_;
  std::string name_;
  std::unordered_map<std::string, Table<StorageType> *> tables_;
  std::vector<std::string> table_names_;
  Spliter spliter_;
};

template <typename StorageType>
RC Db<StorageType>::init()
{
  // open all table
  // db path
  std::string db_path = base_dir_ + "/" + "db_" + name_ + ".json";
  std::vector<std::string> db_tables;
  auto db_meta_v = parse_file2json(db_path);
  if (!db_meta_v.is_object()) {
    return RC::DB_OPEN_TABLE_ERROR;
  }
  auto &&db_meta = db_meta_v.as_object();

  auto tables = db_meta.at("tables");

  if (!tables.is_array()) {
    return RC::DB_OPEN_TABLE_ERROR;
  }

  auto &&tables_array = tables.as_array();
  for (auto &&table : tables_array) {
    if (!tables.is_string()) {
      return RC::DB_OPEN_TABLE_ERROR;
    }

    auto &&table_name = table.as_string();
    auto rc = open_table(std::string_view{table_name.c_str(), table_name.size()});
    if (!rc_success(rc)) {
      return rc;
    }
  }

  return RC::SUCCESS;
}

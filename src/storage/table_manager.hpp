/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-16 11:01:47
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-07 17:06:52
 * @FilePath: /tadis/src/storage/db2.hpp
 * @Description: Db的实现
 */
#pragma once
#include "common/rc.hpp"
#include "common/utility.hpp"
#include "common/json.hpp"
#include "storage/table.hpp"
#include "storage/table.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/path_traits.hpp>
#include <boost/proto/detail/remove_typename.hpp>
#include <memory>
#include <regex>
#include <string_view>
#include <vector>

class TableManager {
public:
  friend class TableManagerTester;

  RC init(std::string_view path);

  // TODO flush all metas , flush all tables (bfp destructer)
  void close()
  {}

  // 这里contain参数不用string_view原因是 tables_查询需要类型string，则用view需要创建一个string，会有较大的开销
  bool contain(const std::string &table_name)
  {
    return tables_.find(table_name) != tables_.end();
  }

  bool contain(std::string_view table_name)
  {
    return tables_.find(std::string{table_name.data(), table_name.size()}) != tables_.end();
  }

  RC create_table(const std::string &table_name, TableMeta meta);

  // RC remove_table(std::string_view table_name);

private:
  // check meta filename
  bool check_filename(std::string_view meta_filename);

  // From meta file
  RC open_table(std::string_view meta_filename);

  std::string base_dir_;
  std::unordered_map<std::string, std::unique_ptr<Table>> tables_;
};

inline RC TableManager::init(std::string_view path_str)
{
  using namespace boost;
  base_dir_ = path_str;
  boost::filesystem::path path{path_str};
  // 便利path底下所有文件
  for (auto &&entry : boost::filesystem::directory_iterator(path)) {
    if (filesystem::is_regular_file(entry)        // 是一个文件
        && check_filename(entry.path().string())  // 检查文件名
    ) {
      auto &&table_meta_filename = entry.path().string();
      if (auto rc = open_table(std::string_view{table_meta_filename.data(), table_meta_filename.size()});
          !rc_success(rc)) {
        return rc;
      }
    }
  }
  return RC::SUCCESS;
}

// ${base_dir}/table_${name}_meta.json
inline bool TableManager::check_filename(std::string_view filename)
{
  std::string_view pattern = "table_\\w+_meta.json";
  std::regex rex{pattern.data()};
  return std::regex_match(filename.data(), rex);
}

inline RC TableManager::open_table(std::string_view filename)
{
  auto json_data = parse_file2json(filename);
  Spliter s;
  s.add_split_ch('_');
  s.next();  // skip 'table'
  auto table_name = s.next();

  TableMeta meta;
  if (auto rc = meta.from_json(json_data); !rc_success(rc)) {
    return rc;
  }

  std::unique_ptr<Table> table{new Table{}};
  table->init(base_dir_, std::move(meta));

  tables_.emplace(std::string{table_name.data(), table_name.size()}, std::move(table));

  return RC::SUCCESS;
}

inline RC TableManager::create_table(const std::string &table_name, TableMeta meta)
{
  if (contain(table_name)) {
    return RC::TABLE_ALREADY_EXISTED;
  }
  std::unique_ptr<Table> table{new Table{}};
  table->init(this->base_dir_, std::move(meta));
  tables_.emplace(std::string{table_name.data(), table_name.size()}, std::move(table));
  return RC::SUCCESS;
}
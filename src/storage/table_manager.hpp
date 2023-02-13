/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-16 11:01:47
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 05:04:13
 * @FilePath: /tadis/src/storage/db2.hpp
 * @Description: Db的实现
 */
#pragma once
#include "common/rc.hpp"
#include "common/utility.hpp"
#include "common/json.hpp"
#include "storage/table.hpp"
#include "storage/io/disk.hpp"
#include "storage/table.hpp"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <regex>
#include <string_view>
#include <vector>

class TableManager {
public:
  friend class TableManagerTester;

  RC init(std::string_view path);

  // TODO flush all metas , flush all tables (bfp close)
  void close()
  {
    for (auto &&i : tables_) {
      auto table = i.second.get();
      auto json = table->table_meta_.to_json();
      auto data = json.print();
      auto filename = make_meta_filename(base_dir_, i.first);
      FILE *f = fopen(filename.c_str(), "w+");
      assert(f);
      fwrite(data.data(), data.size(), 1, f);
      fclose(f);

      table->bfp_->close();
    }
  }

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

  // make sure use contain(...) first!
  Table *table(const std::string &name)
  {
    return tables_.at(name).get();
  }

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
  using namespace std;
  base_dir_ = path_str;
  std::filesystem::path path{path_str};
  // 便利path底下所有文件
  for (auto &&entry : std::filesystem::directory_iterator(path)) {
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
  std::string_view pattern = ".tadis/table_\\w+_meta.json";
  std::regex rex{pattern.data()};
  return std::regex_match(filename.data(), rex);
}

inline RC TableManager::open_table(std::string_view filename)
{
  auto json_data = parse_file2json(filename);

  auto last = filename.find_last_of("_");
  auto first = filename.find_first_of("_");
  auto table_name = std::string_view{filename.data() + first + 1, last - first - 1};

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
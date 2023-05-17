#pragma once

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <regex>

#include "catalog/schema.hpp"
#include "common/json.hpp"
#include "sql/parser/ast.hpp"
#include "common/noncopyable.hpp"
#include "storage/table_heap/table_heap.hpp"

// only manage the db file, meta files should managed by catalog
class StorageManager {
  friend class Catalog;
public:
  StorageManager() = default;
  ~StorageManager() {
    for (auto& i : tables_) {
      i.second->close();
    }
  }
  StorageManager(const StorageManager &) = delete;
  StorageManager &operator=(const StorageManager &) = delete;


  // If table already existed (doesn't open), will open it,
  // else create a new table.
  void create_table(const std::string &table_name)
  {
    assert(tables_.find(table_name) == tables_.end());
    std::string db_name = table_name + ".db";
    std::unique_ptr<TableHeap> th{new TableHeap{db_name}};
    tables_.emplace(table_name, std::move(th));
  }

  // drop table and remove files(meta and db)
  void drop_table(const std::string &table_name)
  {
    std::string db_name = table_name + ".db";
    tables_.erase(table_name);
    remove(db_name.c_str());
  }

  TableHeap *get_table(const std::string &table_name)
  {
    auto it = tables_.find(table_name);
    if (it == tables_.end()) {
      return nullptr;
    }
    return it->second.get();
  }

private:
  std::map<std::string, std::unique_ptr<TableHeap>> tables_;
};

class Table {
  friend class Catalog;
public:
  Table(std::string name, Schema schema) : table_name_(std::move(name)), schema_(std::move(schema))
  {}

  ~Table()
  {
    close();
  }

  Table(const Table &) = delete;
  Table &operator=(const Table &) = delete;

  void close()
  {

  }

  auto schema() -> Schema &
  {
    return schema_;
  }

  auto name() const -> const std::string& {
    return table_name_;
  }

private:
  std::string table_name_;
  Schema schema_;
};

class Catalog {
    using TablePtr = std::unique_ptr<Table>;

public:
    Catalog() = default;  // for test

    Catalog(std::string base_dir) : base_dir_(std::move(base_dir)) {
      init();
    }

    ~Catalog() {
      close();
    }

    Catalog(const Catalog &) = delete;
    Catalog &operator=(const Catalog &) = delete;

    void create_table(const std::string &table_name, std::vector<Column> &colum)
    {
      Table * t = new Table{table_name, Schema{std::move(colum)}};
      tables_.emplace(table_name, t);
      stm_.create_table(table_name);
    }

    void drop_table(const std::string &table_name)
    {
      assert(has_table(table_name));
      tables_.erase( table_name);
      std::string meta_file = table_name + ".json";
      remove(meta_file.c_str());
      stm_.drop_table(table_name);
    }

    bool has_table(const std::string &table_name)
    {
        return tables_.find(table_name) != tables_.end();
    }

    Table *get_table(const std::string &table_name)
    {
        auto it = tables_.find(table_name);
        if (it == tables_.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    auto scanner(const std::string& table) -> RecordScanner* {
      auto store = stm_.get_table(table);
      if (store == nullptr) return nullptr;
      return store->scanner();
    }

    auto store_manager() -> StorageManager* {
      return &stm_;
    }

    std::vector<std::string> tables_name() {
      std::vector<std::string> res;
      for (auto &i : tables_) {
        res.push_back(i.first);
      }
      return res;
    }

private:
  void init();
  void close();
  bool is_json(const std::string& file, std::string& table_name);
private:
  std::map<std::string, TablePtr> tables_;
  StorageManager stm_;
  std::string base_dir_;
};

inline void Catalog::init()
{
  for (auto& file : std::filesystem::recursive_directory_iterator{base_dir_}) {
    if (file.is_regular_file()) {
      auto filename = file.path().string();
      std::string table_name;
      if (is_json(filename, table_name)) {
        // json file
        auto db = table_name + ".db";
        TableHeap* heap {new TableHeap{db}};
        stm_.tables_.emplace(table_name, heap);
        JsonFileRWer r;

        pson::Value table_meta;
        auto ok = r.read(filename, table_meta);

        if (!ok) {
          throw std::runtime_error("read json file error");
        }

        Schema schema{{}};
        schema.from_json(table_meta);
        tables_.emplace(table_name, std::make_unique<Table>(table_name , std::move(schema)));
      }
    }
  }
}

inline void Catalog::close() {
  for (auto & i : tables_) {
    std::string meta_filename = base_dir_ + "/" + i.first + ".json";
    JsonFileRWer w;
    auto val = i.second->schema_.to_json();
    w.write(meta_filename, val);
  }
}

inline bool Catalog::is_json(const std::string& file, std::string& table_name) {
  std::string pattern = base_dir_ + "/\\w+.json";
  std::regex rex{pattern.data()};
  bool ok = std::regex_match(file.data(), rex);
  if (!ok) {
    return ok;
  }
  auto begin_iter = file.begin();
  for (auto i = 0; i <= base_dir_.size(); ++i) {
    begin_iter++;
  }
  
  for (auto iter = begin_iter; ; ++iter) {
    char ch = *iter;
    if (ch == '.') break;
    table_name.push_back(ch);
  }

  return true;
} 

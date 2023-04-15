#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "catalog/schema.hpp"
#include "sql/parser/ast.hpp"
#include "common/noncopyable.hpp"
#include "storage/table_heap/table_heap.hpp"

// only manage the db file, meta files should managed by catalog
class StorageManager {
public:
  StorageManager() = default;
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
  {}

  auto schema() -> Schema &
  {
    return schema_;
  }

  auto name() const -> const std::string& {}

private:
  std::string table_name_;
  Schema schema_;
};

class Catalog {
    using TablePtr = std::unique_ptr<Table>;

public:
    Catalog() = default;
    Catalog(const Catalog &) = delete;
    Catalog &operator=(const Catalog &) = delete;

    void create_table(const std::string &table_name, std::vector<Column> &colum)
    {
      Table * t = new Table{table_name, Schema{std::move(colum)}};
      tables_.emplace(table_name, t);
      stm_.create_table(table_name);
    }

    void drop_table(const std::string &table_name)
    {}

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

private:
  std::map<std::string, TablePtr> tables_;
  StorageManager stm_;
};
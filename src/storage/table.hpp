/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:03:29
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 12:21:14
 * @FilePath: /tadis/src/storage/table.hpp
 * @Description: Table
 */
#pragma once
#include "common/rc.hpp"
#include "common/utility.hpp"
#include "common/pson.hpp"
#include "common/noncopyable.hpp"

#include "sql/parser/ast.hpp"
#include "storage/io/buffer_pool.hpp"
#include "storage/io/iodef.hpp"
#include "storage/io/record.hpp"
#include "storage/kv/storage.hpp"
#include "storage/tuple.hpp"

#include <any>
#include <cassert>
#include <cstddef>
#include <memory>
#include <string>

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

  auto tuple_meta() -> TupleMeta &
  {
    return meta_;
  }

  const std::string &name()
  {
    return name_;
  }

  RC from_json(pson::Value &v);
  pson::Value to_json();

private:
  std::string name_;
  TupleMeta meta_;
};

class Table : public noncopyable {
public:
  friend class TableManager;
  friend class TableScanOp;
  friend class TableTester;

  void init(std::string_view dir, TableMeta meta)
  {
    dir_path_ = dir;
    db_filename_ = make_data_filename(dir, meta.name_);
    table_meta_ = std::move(meta);
    bfp_ = std::unique_ptr<BufferPool>{new BufferPool{std::string_view{db_filename_.data(), db_filename_.size()}}};
  }

  // 如果cols是nullptr就表示values是整个行
  RC insert(const std::vector<std::string> *cols, const std::vector<Value> &values)
  {
    auto pid = bfp_->current_pid();
    Page *page = nullptr;
    RC rc = RC::SUCCESS;
    if (pid == 0) {
      page = bfp_->new_page(pid);
    } else {
      page = bfp_->fetch(pid);
    }

    if (!page) {
      return RC::FETCH_PAGE_ERROR;
    }

    rp_.init(page);
    Record rec;

    if (cols == nullptr) {
      rc = make_record(table_meta_.meta_, values, rec);
      if (!rc_success(rc)) {
        return rc;
      }
    } else {
      // TODO
      assert(false);
    }

    RecordId rid;
    bool ok = rp_.insert(rec, rid);
    if (!ok) {
      bfp_->unpin(pid, false);
      page = bfp_->new_page(pid);
      if (!page) {
        return RC::FETCH_PAGE_ERROR;
      }
      rp_.init(page);
      ok = rp_.insert(rec, rid);

      if (!ok) {
        return RC::RECORD_TOO_LARGE;
      }
    }

    bfp_->unpin(pid, true);

    return RC::SUCCESS;
  }

  // No filter
  std::vector<Tuple> tuples()
  {
    RecordScanner scanner;
    std::vector<Tuple> tuples;
    scanner.init(bfp_.get());

    while (scanner.has_next()) {
      Record rec;
      RecordId rid;
      Tuple t;
      scanner.next(rec, rid);
      rec.rid() = rid;
      t.init(&table_meta_.meta_, std::move(rec.data()));
      tuples.emplace_back(std::move(t));
    }

    return tuples;
  }

  // TODO
  RC remove()
  {
    return RC::SUCCESS;
  }

  auto table_meta() -> TableMeta &
  {
    return table_meta_;
  }

  // check values's types compling with the table's colum type
  bool check(const std::vector<Value> &values)
  {
    if (values.size() != table_meta_.meta_.cells_.size()) {
      return false;
    }
    auto &tm = table_meta_.meta_.cells_;
    for (size_t i = 0; i < values.size(); ++i) {
      bool ok = tm[i].check(values[i].type_);
      if (!ok) {
        return ok;
      }
    }
    return true;
  }

private:
  TableMeta table_meta_;
  std::string dir_path_;
  std::string db_filename_;
  RecordPage rp_;
  std::unique_ptr<BufferPool> bfp_ = nullptr;
};

inline RC TableMeta::from_json(pson::Value &v)
{

  if (!v.is_object()) {
    return RC::JSON_DESERIALIZATION_ERROR;
  }

  auto &&obj = v.as<pson::Object>();

  auto ok = obj.has("name");
  if (ok == false) {
    LOG_DEBUG << "can't find name_";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  auto &&name_v = obj["name"];
  if (!name_v.is_string()) {
    LOG_DEBUG << "name_v's type isn't string";
    return RC::JSON_DESERIALIZATION_ERROR;
  }

  name_ = name_v.as_string();

  ok = obj.has("meta");

  if (ok == false) {
    LOG_DEBUG << "can't find meta_";
    return RC::OUT_OF_RANGE;
  }

  auto &&meta_v = obj["meta"];
  if (auto rc = meta_.from_json(meta_v); !rc_success(rc)) {
    return rc;
  }

  return RC::SUCCESS;
}

inline pson::Value TableMeta::to_json()
{
  pson::Value res{pson::JSON_TYPE::JSON_OBJECT};
  auto &&obj = res.as_object();
  obj.insert(std::string{"name"}, pson::Value{name_});
  obj.insert(std::string{"meta"}, meta_.to_json());
  return res;
}

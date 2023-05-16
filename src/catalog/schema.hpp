#pragma once

#include <vector>

#include "catalog/column.hpp"

class Schema {
public:
  Schema(std::vector<Column> cols) : columns_(std::move(cols))
  {}

  bool operator== (const Schema& other) {
    if (columns_.size() != other.columns_.size()) {
      return false;
    }

    for (auto i = 0; i < columns_.size(); ++i) {
      if (!(columns_[i] == other.columns_[i])) {
        return false;
      }
    }

    return true;
  }

  const std::vector<Column> &columns() const
  {
    return columns_;
  }

  int column_idx(const std::string& col_name) const {
    for (auto i = 0; i < columns_.size(); ++i) {
      if (columns_[i].name_ == col_name) {
        return i;
      }
    }

    return -1;
  }

  void copy_schema_from(Schema* other, const std::vector<size_t> &attr) {
    this->columns_.clear();
    for (auto i : attr) {
      columns_.push_back(other->columns_[i]);
    }
  }

  static Schema copy_schema(const Schema *other, const std::vector<size_t> &attr)
  {
    std::vector<Column> cols;
    for (auto i : attr) {
      cols.push_back(other->columns_[i]);
    }
    return Schema(cols);
  }

  pson::Value to_json() const;
  void from_json(pson::Value& v);

  std::vector<Column> columns_;
};

inline pson::Value Schema::to_json() const
{
    pson::Value v{pson::JSON_TYPE::JSON_OBJECT};
    pson::Value cols{pson::JSON_TYPE::JSON_ARRAY};
    for (auto &col : columns_) {
      cols.push_back(col.to_json());
    }
    v.insert("columns", cols);
    return v;
}

inline void Schema::from_json(pson::Value &v)
{
  assert(v.is_object());
  auto cols = v["columns"];
  auto array = cols.as_array();
  for (auto &col : array.values()) {
    Column c{ColumnType::UNKNOWN, "", 0};
    c.from_json(col);
    columns_.push_back(c);
  }
}
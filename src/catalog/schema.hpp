#pragma once

#include <vector>

#include "catalog/column.hpp"

class Schema {
public:
  Schema(std::vector<Column> cols) : columns_(std::move(cols))
  {}

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

  std::vector<Column> columns_;
};
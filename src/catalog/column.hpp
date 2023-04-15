#pragma once

#include <string>

#include "sql/parser/ast.hpp"
#include "sql/stmt.hpp"

enum class ColumnType : int { INT, FLOAT, VARCHAR, CHAR, UNKNOWN };

class Column {
  friend class Schema;

public:
  Column(ColumnType type, std::string name, size_t size) : type_(type), name_(name), size_(size)
  {}

  // Getter
  ColumnType type() const
  {
    return type_;
  }
  const std::string &name() const
  {
    return name_;
  }
  size_t size() const
  {
    return size_;
  }

public:
  static ColumnType column_from_string(std::string_view t)
  {
    if (t == "int") {
      return ColumnType::INT;
    } else if (t == "float") {
      return ColumnType::FLOAT;
    } else if (t == "varchar") {
      return ColumnType::VARCHAR;
    } else if (t == "char") {
      return ColumnType::CHAR;
    } else {
      return ColumnType::UNKNOWN;
    }
  }

private:
  ColumnType type_ = ColumnType::UNKNOWN;
  std::string name_ = "";
  size_t size_ = 0;
};
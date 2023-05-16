#pragma once

#include <string>

#include "sql/parser/ast.hpp"
#include "sql/stmt.hpp"
#include "common/json.hpp"

enum class ColumnType : int { INT, FLOAT, VARCHAR, CHAR, UNKNOWN };

inline bool check_attr_type(AttrType attr_type, ColumnType col_type) {
  switch (col_type) {
    case ColumnType::INT:
        return AttrType::INTS == attr_type;
    case ColumnType::FLOAT:
        return AttrType::FLOATS == attr_type;
    case ColumnType::VARCHAR:
    case ColumnType::CHAR:
        return AttrType::STRING == attr_type;
    case ColumnType::UNKNOWN :
      return false;
  }
}

class Column {
  friend class Schema;

public:

  Column(ColumnType type, std::string name, size_t size) : type_(type), name_(name), size_(size)
  {}

  bool operator== (const Column& other) {
    return type_ == other.type_ && name_ == other.name_ && size_ == other.size_;
  }

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

  pson::Value to_json() const;
  void from_json(pson::Value& v);
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

inline pson::Value Column::to_json() const
{
    pson::Value v{pson::JSON_TYPE::JSON_OBJECT};
    v.insert("name", name_);
    v.insert("type", (double)static_cast<int>(type_));
    v.insert("size", (double)size_);
    return v;
}

inline void Column::from_json( pson::Value &v)
{
  assert(v.is_object());
  pson::Object &obj = v.as_object();
  assert(obj.has("name"));
  assert(obj.has("type"));
  assert(obj.has("size"));

  auto name_v = obj.at("name");
  auto type_v = obj.at("type");
  auto size_v = obj.at("size");

  assert(name_v.is_string());
  assert(type_v.is_number());
  assert(size_v.is_number());

  int type_int = (int)type_v.as_number();

  name_ = name_v.as_string();
  size_ = (size_t)size_v.as_number();
  type_ = static_cast<ColumnType>(type_int);
}
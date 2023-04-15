#pragma once


#include "catalog/column.hpp"
#include "catalog/schema.hpp"
#include "sql/parser/ast.hpp"
#include "sql/stmt.hpp"
#include "storage/storage_def.hpp"

#include <any>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <sstream>
#include <vector>

class Tuple {
  friend class Storage;

public:
  Tuple(std::vector<char> data) : data_(std::move(data))
  {}

  Tuple(const Tuple & other) {
    data_ = other.data_;
  }
  Tuple &operator=(const Tuple &other) {
      data_ = other.data_;
      return *this;
  };

  static Tuple create_tuple(const std::vector<Value> &vals, Schema *schema);

  const char *data() const
  {
    return data_.data();
  }

  size_t len() const
  {
    return data_.size();
  }

  std::string to_string(Schema *s)
  {
    std::stringstream ss;
    for (auto i = 0; i < s->columns().size(); ++i) {
      auto val = value_at(s, i);
      ss << val.to_string() << " ";
    }
    return ss.str();
  }

  Value value_at(Schema *schema, size_t idx)
  {
    assert(schema && idx < schema->columns().size() && !data_.empty());
    Value res;
    auto &columns = schema->columns();
    auto data = data_.data();
    size_t offset = 0;
    int size = 0;  // for varchar
    for (auto i = 0; i <= columns.size(); ++i) {
      if (i == idx) {
        if (columns[i].type() == ColumnType::CHAR) {
          size = columns[i].size();
        }
        if (columns[i].type() == ColumnType::VARCHAR) {
          memcpy(&size, data, sizeof(int));
          data += sizeof(int);
        }

        init_value(res, data, size, columns[i].type());
        break;
      }

      // skip current value
      switch (columns[i].type()) {
        case ColumnType::INT:
            data += sizeof(int);
            break;
        case ColumnType::FLOAT:
            data += sizeof(float );
            break;
        case ColumnType::CHAR:
          data += columns[i].size();
          break;
        case ColumnType::VARCHAR:
          memcpy(&size, data, sizeof(int));
          data += sizeof(int) + size;
          break;
        default:
          assert(false && "unknown column type");
          break;
      }

      offset = data - data_.data();
      assert(offset <= data_.size());
    }

    return res;
  }

  std::vector<char> data_;
  RID rid_;

private:
  void init_value(Value &val, char *data, int size, ColumnType type)
  {
    switch (type) {
      case ColumnType::INT:
        int num;
        memcpy(&num, data, sizeof(int));
        // val.value_ = num;
        // val.type_ = AttrType::INTS;
        val.init(num);
        break;
      case ColumnType::FLOAT:
        float f;
        memcpy(&f, data, sizeof(float));
        // val.value_ = f;
        // val.type_ = AttrType::FLOATS;
        val.init(f);
        break;
      case ColumnType::CHAR:
      case ColumnType::VARCHAR: {
        std::string s;
        s.assign(data, size);
        val.init(std::move(s));
      } break;
      default:
        assert(false && "unknown column type");
        break;
    }
  }

private:
};

inline Tuple Tuple::create_tuple(const std::vector<Value> &vals, Schema *schema)
{
  std::vector<char> record;
  size_t idx = 0;
  for (auto &v : vals) {
    // check type first
    switch (v.type_) {
      case AttrType::INTS:
        assert(schema->columns()[idx].type() == ColumnType::INT);
        break;
      case AttrType::FLOATS:
        assert(schema->columns()[idx].type() == ColumnType::FLOAT);
        break;
      case AttrType::STRING:
        assert(
            schema->columns()[idx].type() == ColumnType::VARCHAR || schema->columns()[idx].type() == ColumnType::CHAR);
        break;
      default:
        assert(false && "unknown column type");
        break;
    }

    switch (v.type_) {
      case AttrType::INTS: {
        int n = std::any_cast<int>(v.value_);
        record.insert(record.end(), (char *)&n, (char *)&n + sizeof(int));
        break;
      }
      case AttrType::FLOATS: {
        float n = std::any_cast<float>(v.value_);
        record.insert(record.end(), (char *)&n, (char *)&n + sizeof(float));
        break;
      }
      case AttrType::STRING: {
        std::string s = std::any_cast<std::string>(v.value_);
        if (schema->columns()[idx].type() == ColumnType::VARCHAR) {
          int size = s.size();
          record.insert(record.end(), (char *)&size, (char *)&size + sizeof(int));
        }
        for (auto ch : s) {
          record.push_back(ch);
        }
        break;
      }
      default:
        assert(false && "unknown column type");
        break;
    }
    idx++;
  }
  return Tuple{std::move(record)};
}
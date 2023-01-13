/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:03:36
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-13 12:51:08
 * @FilePath: /tadis/src/storage/tuple.hpp
 */
#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <common/utility.hpp>
#include <initializer_list>
#include <memory>
#include <storage/kv/storage.hpp>
#include <string_view>
#include <type_traits>

// 字符串存放在二进制中时， 前4个byte是该字符串的长度.

enum class TupleCellType { UNKNOW, FLOAT, INTEGER, VARCHAR, CHAR };

class TupleCellMeta {
public:
  std::string name_;
  TupleCellType type_ = TupleCellType::UNKNOW;
  size_t len_ = 0;  // for char

  static TupleCellMeta init(std::string_view name, TupleCellType type)
  {
    return {std::string(name), type, 0};
  }

  static TupleCellMeta init(std::string_view name, TupleCellType type, size_t len)
  {
    return {std::string(name), type, len};
  }
};

class TupleMeta {
public:
  std::vector<TupleCellMeta> cells_;

  TupleMeta() = default;
  ~TupleMeta() = default;
  TupleMeta(std::initializer_list<TupleCellMeta> l) : cells_(l.begin(), l.end())
  {}

  static TupleMeta init(std::vector<TupleCellMeta> cells)
  {
    TupleMeta meta;
    meta.cells_ = std::move(cells);
    return meta;
  }
};

class TupleCell {
public:
  TupleCell() : type_(TupleCellType::UNKNOW)
  {}

  TupleCell(BytesView cell_rec, TupleCellType type) : type_(type), cell_record_(cell_rec)
  {}

  void init(BytesView cell_rec, TupleCellType type)
  {
    type_ = type;
    cell_record_ = cell_rec;
  }

  auto type() const
  {
    return type_;
  }

  auto record() const
  {
    return cell_record_;
  }

  auto as_integer()
  {
    assert(type_ == TupleCellType::INTEGER && cell_record_.size() == sizeof(long));
    long res{0};
    memcpy(&res, cell_record_.data(), sizeof(long));
    return res;
  }

  auto as_float()
  {
    assert(type_ == TupleCellType::FLOAT && cell_record_.size() == sizeof(float));
    float res{0.0};
    memcpy(&res, cell_record_.data(), sizeof(float));
    return res;
  }

  auto as_str()
  {
    assert(type_ == TupleCellType::CHAR || type_ == TupleCellType::VARCHAR);
    std::string res;
    for (auto ch : cell_record_) {
      res.push_back(ch);
    }
    return res;
  }

  auto as_str_view()
  {
    assert(type_ == TupleCellType::CHAR || type_ == TupleCellType::VARCHAR);
    std::string_view res{reinterpret_cast<char *>(cell_record_.data()), cell_record_.size()};
    return res;
  }

  auto record()
  {
    return cell_record_;
  }

  std::string to_string()
  {
    std::string res;
    switch (type_) {
      case TupleCellType::VARCHAR:
      case TupleCellType::CHAR: {
        for (auto ch : cell_record_)
          res.push_back(static_cast<char>(ch));
      } break;
      case TupleCellType::INTEGER: {
        res.append(std::to_string(as_integer()));
      } break;
      case TupleCellType::FLOAT: {
        res.append(std::to_string(as_float()));
      } break;

      default: {
        res.append("Unknow cell type");
      } break;
    }
    return res;
  }

private:
  TupleCellType type_;
  BytesView cell_record_;
};

class Tuple {
public:
  Tuple()
  {}

  Tuple(TupleMeta *m, Bytes rec)
  {
    init(m, rec);
  }

  void init(TupleMeta *m, Bytes rec)
  {
    meta_ptr_ = m;
    record_ = std::move(rec);
  }

  RC get_cell(size_t idx, TupleCell &c)
  {
    assert(meta_ptr_ != nullptr || idx < meta_ptr_->cells_.size());
    uint8_t *start{record_.data()};
    size_t len{0};
    TupleCellType t{TupleCellType::UNKNOW};
    for (auto i = 0; i < meta_ptr_->cells_.size(); ++i) {
      auto &&cell = meta_ptr_->cells_[i];
      size_t cell_len = 0;
      switch (cell.type_) {
        case TupleCellType::VARCHAR: {
          uint32_t str_len{0};
          memcpy(&str_len, start, sizeof str_len);
          start += sizeof str_len;
          cell_len = str_len;
          len = cell_len;
          t = TupleCellType::VARCHAR;
        } break;
        case TupleCellType::CHAR: {
          cell_len = cell.len_;
          len = cell.len_;
          t = TupleCellType::CHAR;
        } break;
        case TupleCellType::INTEGER: {
          cell_len = sizeof(long);
          len = sizeof(long);
          t = TupleCellType::INTEGER;
        } break;
        case TupleCellType::FLOAT: {
          cell_len = sizeof(float);
          len = sizeof(float);
          t = TupleCellType::FLOAT;
        } break;
        default: {
          assert(false);
        }
      }
      if (i == idx) {
        break;
      }
      start += cell_len;
    }

    if (t == TupleCellType::UNKNOW) {
      c = TupleCell{};
      return RC::TUPLE_CELL_NOT_EXIST;
    }

    BytesView b{start, len};
    c = TupleCell(b, t);
    return RC::SUCCESS;
  }

  RC get_cell(const std::string &name, TupleCell &c)
  {
    uint8_t *start{record_.data()};
    size_t len{0};
    TupleCellType t{TupleCellType::UNKNOW};

    for (auto &&cell : meta_ptr_->cells_) {
      size_t cell_len = 0;
      switch (cell.type_) {
        case TupleCellType::VARCHAR: {
          uint32_t str_len{0};
          memcpy(&str_len, start, sizeof str_len);
          start += sizeof str_len;
          cell_len = str_len;
          len = cell_len;
          t = TupleCellType::VARCHAR;
        } break;
        case TupleCellType::CHAR: {
          cell_len = cell.len_;
          len = cell.len_;
          t = TupleCellType::CHAR;
        } break;
        case TupleCellType::INTEGER: {
          cell_len = sizeof(long);
          len = sizeof(long);
          t = TupleCellType::INTEGER;
        } break;
        case TupleCellType::FLOAT: {
          cell_len = sizeof(float);
          len = sizeof(float);
          t = TupleCellType::FLOAT;
        } break;
        default: {
          assert(false);
        }
      }
      if (cell.name_ == name) {
        BytesView b{start, len};
        c = TupleCell{b, t};
        return RC::SUCCESS;
      }
      start += cell_len;
    }
    c = TupleCell{};
    return RC::TUPLE_CELL_NOT_EXIST;
  }

private:
  TupleMeta *meta_ptr_ = nullptr;
  Bytes record_;
};

/**
 *@brief 把float, long encode
 */
template <typename T>
inline void encode_num(Bytes &bytes, T t)
{
  static_assert(
      std::is_same<T, float>::value || std::is_same<T, long>::value, "encode2bytes only support float and long");
  unsigned char *start = reinterpret_cast<unsigned char *>(&t);
  for (auto i = 0; i < sizeof t; ++i) {
    bytes.push_back(start[i]);
  }
}

/**
 *@brief  var-len string encode
 */
template <typename T>
inline void encode_varchar(Bytes &bytes, const T &s)
{
  static_assert(std::is_same<T, std::string>::value || std::is_same<T, std::string_view>::value,
      "encode2bytes only support float and long");
  uint32_t size = s.size();
  unsigned char *start = reinterpret_cast<unsigned char *>(&size);
  for (auto i = 0; i < sizeof size; ++i) {
    bytes.push_back(start[i]);
  }
  for (auto ch : s) {
    bytes.push_back(ch);
  }
}

/**
 *@brief const-len string encode
 */
template <typename T>
inline void encode_char(Bytes &bytes, const T &s)
{
  static_assert(std::is_same<T, std::string>::value || std::is_same<T, std::string_view>::value,
      "encode2bytes only support float and long");
  for (auto ch : s) {
    bytes.push_back(ch);
  }
}
/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:03:36
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-11 20:06:26
 * @FilePath: /tadis/src/storage/tuple.hpp
 */
#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <common/utility.hpp>
#include <memory>
#include <storage/table.hpp>
#include <storage/kv/storage.hpp>

// 字符串存放在二进制中时， 前4个byte是该字符串的长度.

enum class TupleCellType { UNKNOW, FLOAT, INTEGER, VARCHAR, CHAR };

class TupleCellMeta {
public:
  std::string name_;
  TupleCellType type_ = TupleCellType::UNKNOW;
  size_t len_ = 0;  // for char
};

class TupleMeta {
public:
  std::vector<TupleCellMeta> cells_;
};

class TupleCell {
public:
  TupleCell() : type_(TupleCellType::UNKNOW)
  {}

  TupleCell(BytesView cell_rec, TupleCellType type) : type_(type), cell_record_(cell_rec)
  {}

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
  void init(TupleMeta *m, Bytes rec)
  {
    meta_ptr_ = m;
    record_ = std::move(rec);
  }

  TupleCell get_cell(size_t idx)
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
          t = TupleCellType::UNKNOW;
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
    BytesView b{start, len};
    return TupleCell(b, t);
  }

  TupleCell get_cell(const std::string &name)
  {
    uint8_t *start{nullptr};
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
          t = TupleCellType::UNKNOW;
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
        return {b, t};
      }
      start += cell_len;
    }

    return TupleCell{};
  }

private:
  TupleMeta *meta_ptr_ = nullptr;
  Bytes record_;
};
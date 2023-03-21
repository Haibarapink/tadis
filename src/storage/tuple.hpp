/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:03:36
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 04:12:20
 * @FilePath: /tadis/src/storage/tuple.hpp
 */
#pragma once

#include "common/bytes.hpp"
#include "common/expect.hpp"
#include "common/logger.hpp"
#include "common/rc.hpp"
#include "common/json.hpp"
#include "common/utility.hpp"
#include "storage/io/record.hpp"
#include "sql/parser/ast.hpp"
#include "tuple_utility.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <initializer_list>
#include <memory>
#include <sstream>
#include <string_view>
#include <type_traits>

// 字符串存放在二进制中时， 前4个byte是该字符串的长度.
enum class TupleCellType { FLOAT = 0, INTEGER, VARCHAR, CHAR, UNKNOW };

class TupleCellMeta {
public:
  std::string name_;
  TupleCellType type_ = TupleCellType::UNKNOW;
  size_t len_ = 0;  // for char
  bool visible_ = false;

  TupleCellMeta()
  {}
  ~TupleCellMeta()
  {}

  TupleCellMeta(const TupleCellMeta &other)
  {
    copy(other);
  }

  TupleCellMeta &operator=(const TupleCellMeta &other)
  {
    copy(other);
    return *this;
  }

  TupleCellMeta(TupleCellMeta &&other)
  {
    move_from(std::move(other));
  }

  TupleCellMeta &operator=(TupleCellMeta &&other)
  {
    move_from(std::move(other));
    return *this;
  }

  void copy(const TupleCellMeta &other)
  {
    this->name_ = other.name_;
    this->type_ = other.type_;
    this->len_ = other.len_;
    this->visible_ = other.visible_;
  }

  void move_from(TupleCellMeta &&other)
  {
    this->name_ = std::move(other.name_);
    this->type_ = other.type_;
    this->len_ = other.len_;
    this->visible_ = other.visible_;
    other.visible_ = false;
    other.len_ = 0;
    other.type_ = TupleCellType::UNKNOW;
  }

  static TupleCellMeta init(std::string_view name, TupleCellType type)
  {
    TupleCellMeta tm;
    tm.name_ = name;
    tm.type_ = type;
    tm.len_ = 0;
    return tm;
  }

  static TupleCellMeta init(std::string_view name, TupleCellType type, size_t len)
  {
    TupleCellMeta tm;
    tm.name_ = name;
    tm.type_ = type;
    tm.len_ = len;
    return tm;
  }

  static TupleCellMeta init(const ColAttr &col)
  {
    TupleCellMeta tm;

    tm.name_ = col.name_;
    tm.len_ = col.size_;

    if (col.type_ == "VARCHAR") {
      tm.type_ = TupleCellType::VARCHAR;
    } else if (col.type_ == "CHAR") {
      tm.type_ = TupleCellType::CHAR;
    } else if (col.type_ == "INT") {
      tm.type_ = TupleCellType::INTEGER;
    } else if (col.type_ == "FLOAT") {
      tm.type_ = TupleCellType::FLOAT;
    } else {
      assert(false);
    }
    return tm;
  }

  bool check(const AttrType &type)
  {
    switch (type_) {
      case TupleCellType::VARCHAR:
      case TupleCellType::CHAR:
        return type == AttrType::STRING;
      case TupleCellType::FLOAT:
        return type == AttrType::FLOATS;
      case TupleCellType::INTEGER:
        return type == AttrType::INTS;
      default:
        return false;
    }
  }

  pson::Value to_json();
  RC from_json(pson::Value &v);
};

class TupleMeta {
public:
  std::vector<TupleCellMeta> cells_;

  TupleMeta()
  {}
  ~TupleMeta()
  {}
  TupleMeta(const TupleMeta &other)
  {
    cells_ = other.cells_;
  }
  TupleMeta &operator=(const TupleMeta &other)
  {
    cells_ = other.cells_;
    return *this;
  }
  TupleMeta(TupleMeta &&other)
  {
    cells_ = std::move(other.cells_);
  }
  TupleMeta &operator=(TupleMeta &&other)
  {
    cells_ = std::move(other.cells_);
    return *this;
  }

  TupleMeta(std::initializer_list<TupleCellMeta> l) : cells_(l.begin(), l.end())
  {}

  static TupleMeta init(std::vector<TupleCellMeta> cells)
  {
    TupleMeta meta;
    meta.cells_ = std::move(cells);
    return meta;
  }

  pson::Value to_json();
  RC from_json(pson::Value &v);
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

  TupleCell(const TupleCell &other)
  {
    copy(other);
  }

  TupleCell &operator=(const TupleCell &other)
  {
    copy(other);
    return *this;
  }

  TupleCell(TupleCell &&other)
  {
    copy(other);
    other.cell_record_ = BytesView{};
    other.type_ = TupleCellType::UNKNOW;
  }

  TupleCell &operator=(TupleCell &&other)
  {
    copy(other);
    other.cell_record_ = BytesView{};
    other.type_ = TupleCellType::UNKNOW;
    return *this;
  }

  void copy(const TupleCell &other)
  {
    this->cell_record_ = other.cell_record_;
    this->type_ = other.type_;
  }

  auto type() const
  {
    return type_;
  }

  auto record() const
  {
    return cell_record_;
  }

  auto as_integer() const
  {
    assert(type_ == TupleCellType::INTEGER && cell_record_.size() == sizeof(long));
    long res{0};
    memcpy(&res, cell_record_.data(), sizeof(long));
    return res;
  }

  auto as_float() const
  {
    assert(type_ == TupleCellType::FLOAT && cell_record_.size() == sizeof(float));
    float res{0.0};
    memcpy(&res, cell_record_.data(), sizeof(float));
    return res;
  }

  auto as_str() const
  {
    assert(type_ == TupleCellType::CHAR || type_ == TupleCellType::VARCHAR);
    std::string res;
    for (auto ch : cell_record_) {
      res.push_back(ch);
    }
    return res;
  }

  auto as_str_view() const
  {
    assert(type_ == TupleCellType::CHAR || type_ == TupleCellType::VARCHAR);
    std::string_view res{cell_record_.data(), cell_record_.size()};
    return res;
  }

  auto record()
  {
    return cell_record_;
  }

  std::string to_string() const
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
        assert(false);
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

  Tuple(const Tuple &other)
  {
    copy(other);
  }

  Tuple &operator=(const Tuple &other)
  {
    copy(other);

    return *this;
  }

  Tuple(Tuple &&other)
  {
    move_from(std::move(other));
  }

  Tuple &operator=(Tuple &&other)
  {
    move_from(std::move(other));
    return *this;
  }

  void copy(const Tuple &other)
  {
    this->meta_ptr_ = other.meta_ptr_;
    this->record_ = other.record_;
  }

  void move_from(Tuple &&other)
  {
    this->meta_ptr_ = other.meta_ptr_;
    this->record_ = std::move(other.record_);
    other.meta_ptr_ = nullptr;
  }

  void init(TupleMeta *m, Bytes rec)
  {
    meta_ptr_ = m;
    record_ = std::move(rec);
  }

  auto tuple_meta() -> TupleMeta *
  {
    return this->meta_ptr_;
  }

  RC get_cell(size_t idx, TupleCell &c)
  {
    assert(meta_ptr_);
    if (idx >= meta_ptr_->cells_.size()) {
      return RC::OUT_OF_RANGE;
    }

    char *start{record_.data()};
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

  RC get_cell(std::string_view name, TupleCell &c)
  {
    char *start{record_.data()};
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

  std::string to_string()
  {
    std::string res;

    if (!meta_ptr_) {
      res.append("Empty Tuple!\n");
      return res;
    }
    for (size_t i = 0; i < meta_ptr_->cells_.size(); ++i) {
      auto &&cell_meta = meta_ptr_->cells_[i];
      TupleCell cell;
      auto rc = this->get_cell(i, cell);
      // If get_cell don't return success, it should is a bug.
      assert(rc == RC::SUCCESS);
      res.append(cell_meta.name_);
      res.append(" : ");
      res.append(cell.to_string());
      res.append("\n");
    }

    return res;
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
  char *start = reinterpret_cast<char *>(&t);
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
  char *start = reinterpret_cast<char *>(&size);
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

inline bool compare_type(const TupleCellType &cell_type, const AttrType &attr_type)
{
  switch (cell_type) {
    case TupleCellType::VARCHAR:
    case TupleCellType::CHAR:
      return attr_type == AttrType::STRING;
    case TupleCellType::FLOAT:
      return attr_type == AttrType::FLOATS;
    case TupleCellType::INTEGER:
      return attr_type == AttrType::INTS;
    default:
      return false;
  }
  return false;
}

inline RC make_record(const TupleMeta &meta, const std::vector<Value> &values, Record &rec)
{
  if (meta.cells_.size() != values.size()) {
    return RC::TUPLE_CELL_NOT_EXIST;
  }
  auto &&cell_metas = meta.cells_;
  Record r;
  for (auto i = 0; i < cell_metas.size(); ++i) {
    // 检查类型
    auto &&cell_meta = cell_metas[i];
    auto &&value = values[i];
    if (!compare_type(cell_meta.type_, value.type_)) {
      return RC::TUPLE_CELL_NOT_EXIST;
    }
    if (cell_meta.type_ == TupleCellType::CHAR) {
      encode_char(r.data(), std::any_cast<const std::string &>(value.value_));
    } else if (cell_meta.type_ == TupleCellType::VARCHAR) {
      encode_varchar(r.data(), std::any_cast<const std::string &>(value.value_));
    } else if (cell_meta.type_ == TupleCellType::INTEGER) {
      encode_num(r.data(), std::any_cast<long>(value.value_));
    } else {
      encode_num(r.data(), std::any_cast<float>(value.value_));
    }
  }
  rec.data() = std::move(r.data());
  return RC::SUCCESS;
}

template <typename T>
inline pson::Value to_json(T &t)
{
  return t.to_json();
}

template <typename T>
inline RC from_json(T &t, pson::Value &v)
{
  return t.from_json(v);
}

inline pson::Value TupleCellMeta::to_json()
{
  pson::Value res{pson::JSON_TYPE::JSON_OBJECT};
  auto &&obj = res.as_object();
  obj.insert(std::string{"name"}, name_);
  obj.insert(std::string{"type"}, static_cast<int>(type_));
  obj.insert(std::string{"len"}, len_);
  obj.insert(std::string{"visible"}, visible_);

  return res;
}

inline RC TupleCellMeta::from_json(pson::Value &v)
{
  if (!v.is_object()) {
    LOG_DEBUG << "v is not a json object";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  auto &&obj = v.as_object();

  auto &&name_v = obj.at("name");

  if (!name_v.is_string()) {
    LOG_DEBUG << "name_v's type isn't string";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  name_ = name_v.as_string();

  auto &&type_v = obj.at("type");
  if (!type_v.is_number()) {
    LOG_DEBUG << "type_v's type isn't int64";
    return RC::JSON_DESERIALIZATION_ERROR;
  }

  if ((long)type_v.as_number() > static_cast<int>(TupleCellType::UNKNOW) ||
      (long)type_v.as_number() < static_cast<int>(TupleCellType::FLOAT)) {
    LOG_DEBUG << "type_v " << (long)type_v.as_number() << " out of range of enum class";
    return RC::JSON_DESERIALIZATION_ERROR;
  }

  type_ = static_cast<TupleCellType>((long)type_v.as_number());

  auto &&len_v = obj.at("len");
  if (!len_v.is_number()) {
    LOG_DEBUG << "len_v's type isn't uint64";
    return RC::JSON_DESERIALIZATION_ERROR;
  }

  len_ = (long)len_v.as_number();

  auto &&visible_v = obj.at("visible");
  if (!visible_v.is_bool()) {
    LOG_DEBUG << "visible_v's type isn't bool";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  visible_ = visible_v.as_bool();

  return RC::SUCCESS;
}

inline pson::Value TupleMeta::to_json()
{
  pson::Value res{pson::JSON_TYPE::JSON_OBJECT};
  pson::Object &obj = res.as_object();
  pson::Array cells;
  for (auto &cell : cells_) {
    cells.push_back(cell.to_json());
  }
  obj.insert("cells", pson::Value(cells));

  return res;
}

inline RC TupleMeta::from_json(pson::Value &v)
{
  if (!v.is_object()) {
    LOG_DEBUG << "v isn't a json object";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  auto &&obj = v.as_object();

  auto &&cells_v = obj.at("cells");
  if (!cells_v.is_array()) {
    LOG_DEBUG << "cells_v isn't a json array";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  auto &&cells = cells_v.as_array();
  for (auto i = 0; i < cells.size(); ++i) {
    auto &&cell = cells.at(i);
    TupleCellMeta m;
    auto rc = m.from_json(cell);
    if (!rc_success(rc)) {
      return rc;
    }
    cells_.emplace_back(std::move(m));
  }
  return RC::SUCCESS;
}

// 这里是filter
class Comparator {
public:
  static int cmp(const TupleCell &l, const TupleCell &r)
  {
    assert(l.type() == r.type());
    switch (l.type()) {
      case TupleCellType::VARCHAR:
      case TupleCellType::CHAR:
        return cmp_str(l.as_str_view(), r.as_str_view());
      case TupleCellType::FLOAT:
        return cmp_num(l.as_float(), r.as_float());
      case TupleCellType::INTEGER:
        return cmp_num(l.as_integer(), r.as_integer());
      case TupleCellType::UNKNOW:
        assert(false);
    }

    // make compiler happy
    return -1;
  }

  static int cmp(const Value &l, const TupleCell &r)
  {
    bool check_type = compare_type(r.type(), l.type_);
    assert(check_type);
    switch (r.type()) {
      case TupleCellType::VARCHAR:
      case TupleCellType::CHAR: {
        const std::string &l_str = std::any_cast<const std::string &>(l.value_);
        return cmp_str(std::string_view{l_str.data(), l_str.size()}, r.as_str_view());
      }
      case TupleCellType::FLOAT:
        return cmp_num(std::any_cast<float>(l.value_), r.as_float());
      case TupleCellType::INTEGER:
        return cmp_num(std::any_cast<long>(l.value_), r.as_integer());
      default:
        assert(false);
    }
    // make compiler happy
    return -1;
  }
};

// Check a tuple's cell
class TupleCellFilter {
public:
  friend class TupleFilter;

  TupleCellFilter(Value v, CondOp op) : v_(std::move(v)), op_(op)
  {
    assert(op != CondOp::UNDEFINED);
  }

  TupleCellFilter(TupleCellFilter &&other) : v_(std::move(other.v_)), op_(other.op_)
  {
    other.op_ = CondOp::UNDEFINED;
  }

  TupleCellFilter &operator=(TupleCellFilter &&other)
  {
    v_ = std::move(other.v_);
    op_ = other.op_;
    other.op_ = CondOp::UNDEFINED;
    return *this;
  }

  bool check(const TupleCell &cell)
  {
    int res = Comparator::cmp(v_, cell);

    switch (op_) {
      case CondOp::EQ:
        return res == 0;

      case CondOp::GREATER:
        return res > 0;
      // TODO IN IS ISNOT
      case CondOp::IN:
      case CondOp::IS:
      case CondOp::IS_NOT:
        return false;

      case CondOp::NOT_EQ:
        return res != 0;
      case CondOp::SMALLER:
        return res < 0;
      case CondOp::UNDEFINED:
        assert(false);
        break;
    }
  }

private:
  Value v_;
  CondOp op_;
};

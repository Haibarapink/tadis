/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-06 11:50:03
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-04 17:55:45
 * @FilePath: /tadis/src/sql/sql_define.hpp
 * @Description: sql的定义，比如select ast, insert ast等等
 */

#pragma once

#include "common/rc.hpp"
#include "sql/parser/lexer.hpp"
#include <any>
#include <string>
#include <vector>
#include <variant>

// <Query> ::= SELECT <SelList> FROM <FromList> WHERE <Condition>
// <SelList> ::= <Attribute> , SelList>
// <SelList> ::= <Attribute>

class RelAttr {
public:
  std::string table_;
  std::string attribute_;
};

enum class CondOp {
  UNDEFINED,
  NOT_EQ,   // <>
  EQ,       // =
  GREATER,  // >
  SMALLER,  // <
  IS,       // is (null)
  IS_NOT,   // is not (null)
  IN,
};

inline CondOp token2op(const Token &t)
{
  switch (t) {
    case Token::NOT_EQ_T:
      return CondOp::NOT_EQ;
    case Token::ASSIGN_T:
      return CondOp::EQ;
    case Token::GREATER_T:
      return CondOp::GREATER;
    case Token::SMALLER_T:
      return CondOp::SMALLER;
    case Token::IN_T:
      return CondOp::IN;

    case Token::IS_T:  // TODO 'is not' is two token
      return CondOp::IS;

    default:
      return CondOp::UNDEFINED;
  }
}

// 属性的值类型
enum class AttrType { UNDEFINED, REL_ATTR, STRING, INTS, FLOATS, NULL_A, QUERY };

class Value {
public:
  template <typename T>
  void init(T v, AttrType t)
  {
    value_ = std::move(v);
    type_ = t;
  }

  template <typename T>
  T get()
  {
    return std::any_cast<T>(value_);
  }

  bool check_attr_type(AttrType t) const
  {
    return type_ == t;
  }

  std::any value_;
  AttrType type_ = AttrType::UNDEFINED;

  Value() = default;
  ~Value() = default;
  Value(const Value &v) : value_(v.value_), type_(v.type_)
  {}

  Value &operator=(const Value &v)
  {
    value_ = v.value_;
    type_ = v.type_;
    return *this;
  }

  Value(Value &&v) : value_(std::move(v.value_)), type_(v.type_)
  {
    v.type_ = AttrType::UNDEFINED;
  }

  Value &operator=(Value &&v)
  {
    value_ = std::move(v.value_);
    type_ = v.type_;
    v.type_ = AttrType::UNDEFINED;
    return *this;
  }
};

class Condition {
public:
  Value left_;
  Value right_;
  CondOp op_ = CondOp::UNDEFINED;

  Condition() = default;
  ~Condition() = default;

  Condition(const Condition &c) : left_(c.left_), right_(c.right_), op_(c.op_)
  {}
  Condition &operator=(const Condition &c)
  {
    left_ = c.left_;
    right_ = c.right_;
    op_ = c.op_;

    return *this;
  }

  Condition(Condition &&c) : left_(std::move(c.left_)), right_(std::move(c.right_)), op_(c.op_)
  {
    c.op_ = CondOp::UNDEFINED;
  }

  Condition &operator=(Condition &&c)
  {
    left_ = std::move(c.left_);
    right_ = std::move(c.right_);
    op_ = c.op_;
    c.op_ = CondOp::UNDEFINED;
    return *this;
  }
};

class SelectAst {
public:
  // select list
  std::vector<RelAttr> selist_;
  // from list
  std::vector<std::string> from_list_;
  // condition list
  std::vector<Condition> cond_list_;
};

// DELETE FROM 表名称 WHERE 列名称 = 值
// DELETE FROM 表
// DELETE * FRRM 表
class DeleteAst {
public:
  std::vector<std::string> tables_;
  std::vector<Condition> conds_;
};

class InsertAst {
public:
  std::string table_name_;
  std::vector<std::string> cols_;
  std::vector<Value> values_;
};

class ColAttr {
public:
  std::string name_;
  std::string type_;
  size_t size_ = 0;

  ColAttr() = default;
  ~ColAttr() = default;
  ColAttr(const ColAttr &other) : name_(other.name_), type_(other.type_), size_(other.size_)
  {}

  ColAttr &operator=(const ColAttr &other)
  {
    name_ = other.name_;
    type_ = other.type_;
    size_ = other.size_;
    return *this;
  }

  ColAttr(ColAttr &&other) : name_(std::move(other.name_)), type_(other.type_), size_(other.size_)
  {
    other.size_ = 0;
  }

  ColAttr &operator=(ColAttr &&other)
  {
    name_ = std::move(other.name_);
    type_ = std::move(other.type_);
    size_ = std::move(other.size_);
    other.size_ = 0;
    return *this;
  }
};

// Example:
// CREATE TABLE Persons
// (
// Id_P int,
// LastName varchar(255),
// FirstName varchar(255),
// Address varchar(255),
// City varchar(255)
// )
// 支持类型: float, int, char, varchar
class CreateTableAst {
public:
  std::vector<ColAttr> col_attrs_;
  std::string table_name_;
};
using QueryAst = std::variant<SelectAst, DeleteAst, InsertAst, CreateTableAst>;
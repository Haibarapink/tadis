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

enum class CondOp : int {
  UNDEFINED = 0,
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

  template <typename T>
  Value &operator=(T t)
  {
    init(t);
    return *this;
  }

  template <typename T>
  void init(T t)
  {
    assert(false && "not implemented");
  }

  template <typename T>
  void init(T v, AttrType t)
  {
    value_ = std::move(v);
    type_ = t;
  }

  int compare(Value &v)
  {
    assert(type_ == v.type_);
    int res = 0;

    switch (v.type_) {
      case AttrType::INTS:
        res = get<long>() - v.get<long>();
        break;
      case AttrType::FLOATS:
        res = get<float>() - v.get<float>();
        break;
      case AttrType::STRING:
        res = get<std::string>().compare(v.get<std::string>());
        break;
      default:
        assert(false && "not implemented");
    }

    return res;
  }

  std::string to_string()
  {
    switch (type_) {
      case AttrType::REL_ATTR:
        return get<RelAttr>().table_ + "." + get<RelAttr>().attribute_;
      case AttrType::STRING:
        return get<std::string>();
      case AttrType::INTS:
        return std::to_string(get<long>());
      case AttrType::FLOATS:
        return std::to_string(get<float>());
      case AttrType::NULL_A:
        return "null";
      case AttrType::QUERY:
        // TODO
        return get<std::string>();
      default:
        return "";
    }
  }

  template <typename T>
  T get()
  {
    return std::any_cast<T>(value_);
  }

  template <typename T>
  T &get_ref()
  {
    return std::any_cast<T>(value_);
  }

  bool check_attr_type(AttrType t) const
  {
    return type_ == t;
  }

  std::any value_;
  AttrType type_ = AttrType::UNDEFINED;
};

template <>
inline void Value::init(int n)
{
  init((long)n, AttrType::INTS);
}

template <>
inline void Value::init(float n)
{
  init(n, AttrType::FLOATS);
}

template <>
inline void Value::init(const char *str)
{
  init(std::string(str), AttrType::STRING);
}

template <>
inline void Value::init(std::string v)
{
  init(std::move(v), AttrType::STRING);
}

template <>
inline void Value::init(std::string_view v)
{
  init(std::string(v), AttrType::STRING);
}

template <>
inline void Value::init(RelAttr r)
{
  init(std::move(r), AttrType::REL_ATTR);
}

// anonymous
namespace {
template <typename... Args>
inline void init_values(std::vector<Value> &vals, Args... args)
{}

template <typename T, typename... Args>
inline void init_values(std::vector<Value> &vals, T t, Args... args)
{
  Value v;
  v.init(t);
  vals.push_back(std::move(v));
  init_values(vals, args...);
}
}  // namespace

template <typename... Args>
inline std::vector<Value> init_values(Args... args)
{
  std::vector<Value> res;
  init_values(res, args...);
  return res;
}

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

class SelectStmt {
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
class DeleteStmt {
public:
  std::vector<std::string> tables_;
  std::vector<Condition> conds_;
};

class InsertStmt {
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
class CreateTableStmt {
public:
  std::vector<ColAttr> col_attrs_;
  std::string table_name_;
};

class DropStmt {
public:
  std::string table_;
};

using QueryStmt = std::variant<SelectStmt, DeleteStmt, InsertStmt, CreateTableStmt, DropStmt>;
enum class QueryType : int { SELECT, INSERT, DELETE, CREATE, DROP };
class SqlStmt {
public:
  SqlStmt(QueryStmt q) : q_(std::move(q))
  {
    if (is_select()) {
      type_ = QueryType::SELECT;
    } else if (is_insert()) {
      type_ = QueryType::INSERT;
    } else if (is_delete()) {
      type_ = QueryType::DELETE;
    } else if (is_create_table()) {
      type_ = QueryType::CREATE;
    } else if (is_drop()) {
      type_ = QueryType::DROP;
    }
  }

  QueryType type() const
  {
    return type_;
  }

  bool where_is_vaild()
  {
    auto &select_stmt = stmt<SelectStmt>();
    return !select_stmt.cond_list_.empty();
  }

  template <typename Stmt>
  Stmt &stmt()
  {
    return std::get<Stmt>(q_);
  }

  bool is_select() const
  {
    return std::holds_alternative<SelectStmt>(q_);
  }

  bool is_create_table() const
  {
    return std::holds_alternative<CreateTableStmt>(q_);
  }

  bool is_insert() const
  {
    return std::holds_alternative<InsertStmt>(q_);
  }

  bool is_delete() const
  {
    return std::holds_alternative<DeleteStmt>(q_);
  }

  bool is_drop() const
  {
    return std::holds_alternative<DropStmt>(q_);
  }

private:
  QueryStmt q_;
  QueryType type_;
};

using IntType = long;
/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-06 11:50:03
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-06 16:23:45
 * @FilePath: /tadis/src/sql/sql_define.hpp
 * @Description: sql的定义，比如select ast, insert ast等等
 */

#pragma once

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

// 属性的值类型
enum class AttrType { UNDEFINED, REL_ATTR, CHARS, INTS, FLOATS, NULL_A, QUERY };

class Value {
public:
  //   Value() = default;
  //   ~Value() = default;

  //   Value(const Value &) = default;
  //   Value &operator=(const Value &) = default;
  //   Value(Value &&) = default;
  //   Value &operator=(Value &&) = default;

  template <typename T>
  void init(T v, AttrType t)
  {
    value_ = std::move(v);
    type_ = t;
  }

  std::any value_;
  AttrType type_ = AttrType::UNDEFINED;
};

class Condition {
public:
  Value left_;
  Value right_;
  CondOp op_ = CondOp::UNDEFINED;
};

template <typename... QueriesType>
class TQuery {
public:
  template <typename T>
  bool is()
  {
    return std::holds_alternative<T>(query_);
  }

  // 先调用 is<T>() 来判断类型是否正确
  template <typename T>
  T &as_ref()
  {
    return std::get<T>(query_);
  }

private:
  std::variant<QueriesType...> query_;
};

class Select {
public:
  // select list
  std::vector<RelAttr> selist_;
  // from list
  std::vector<std::string> from_list_;
  // condition list
  std::vector<Condition> cond_list_;
};

using Query = TQuery<Select>;
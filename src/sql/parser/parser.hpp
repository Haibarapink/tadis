/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-06 16:25:58
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-07 23:21:34
 * @FilePath: /tadis/src/sql/parser/parser.hpp
 * @Description: 语法解析
 */
#pragma once

#include "common/rc.hpp"
#include "common/noncopyable.hpp"
#include "common/logger.hpp"
#include "sql/parser/lexer.hpp"
#include "sql/parser/ast.hpp"

#include <any>
#include <cmath>
#include <variant>
#include <vector>

template <typename InputType>
class Parser : public noncopyable {
public:
  Parser(InputType input) : lexer_(input)
  {}

  RC parse();

  const QueryAst &query()
  {
    return query_;
  }

private:
  RC parse_from(std::vector<std::string> &from_list);
  RC parse_value(Value &v);
  RC parse_values(std::vector<Value> &values);
  RC parse_conds(std::vector<Condition> &cond_list);
  RC parse_attrs(std::vector<RelAttr> &attrs);
  RC parse_cols(std::vector<std::string> &cols);
  RC parse_col_attrs(std::vector<ColAttr> &col_attrs);
  RC parse_col_data_type(ColAttr &c);
  // select
  RC parse_select();

  // delete
  RC parse_delete();

  // insert
  RC parse_insert();

  // create
  RC parse_create();
  RC parse_create_table();
  // RC parse_create_index();

  RC parse_drop();

private:
  Lexer<InputType> lexer_;
  QueryAst query_;
};

template <typename InputType>
RC Parser<InputType>::parse()
{
  auto [rc, tk] = lexer_.peek();
  switch (tk) {
    case Token::SELECT_T: {
      return parse_select();
    }
    case Token::CREATE_T: {
      return parse_create();
    }

    case Token::DELETE_T: {
      return parse_delete();
    }

    case Token::INSERT_T: {
      return parse_insert();
    }

    case Token::DROP_T: {
      return parse_drop();
    }
    default: {
      LOG_DEBUG << "Undefined token";
      return RC::SYNTAX_ERROR;
    }
  }
}

template <typename InputType>
RC Parser<InputType>::parse_select()
{
  auto [rc, token] = lexer_.next();
  assert(token == Token::SELECT_T && rc == RC::SUCCESS);

  query_ = SelectAst{};
  auto &select = std::get<SelectAst>(query_);
  if (rc = parse_attrs(select.selist_); rc != RC::SUCCESS) {
    LOG_DEBUG << "FAIL";
    return rc;
  }
  if (rc = parse_from(select.from_list_); rc != RC::SUCCESS) {
    LOG_DEBUG << "FAIL";
    return rc;
  }
  if (rc = parse_conds(select.cond_list_); rc != RC::SUCCESS) {
    LOG_DEBUG << "FAIL";
    return rc;
  }

  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_attrs(std::vector<RelAttr> &attrs)
{
  while (true) {
    // Select * ...
    auto [rc, tk] = lexer_.next();
    if (rc != RC::SUCCESS) {
      LOG_DEBUG << "lexer_.next fail";
      return rc;
    }

    // 解析到 '*'
    if (tk == Token::STAR_T) {
      LOG_DEBUG << "Get START_T";
      RelAttr rel;
      rel.attribute_ = "*";
      if (attrs.size() > 0) {
        return RC::SYNTAX_ERROR;
      }
      attrs.emplace_back(std::move(rel));
      return rc;
    }

    if (tk != Token::ID_T) {
      LOG_DEBUG << "tk isn't ID_T";
      return RC::SYNTAX_ERROR;
    }

    RelAttr rel_attr;
    std::string name;
    name = std::move(lexer_.template cur_ref<std::string>());

    // Peek 下一个 token
    auto peek_res = lexer_.peek();
    rc = peek_res.first;
    tk = peek_res.second;

    if (tk == Token::DOT_T) {
      rel_attr.table_ = std::move(name);
      lexer_.next();
      auto next_res = lexer_.next_if(Token::ID_T);
      if (next_res.first != RC::SUCCESS) {
        return next_res.first;
      }
      rel_attr.attribute_ = std::move(lexer_.template cur_ref<std::string>());
    } else {
      rel_attr.attribute_ = std::move(name);
    }

    LOG_DEBUG << "RelAttr " << rel_attr.table_ << "." << rel_attr.attribute_;

    attrs.emplace_back(std::move(rel_attr));

    auto next_res = lexer_.next_if(Token::COMMAS_T);
    if (next_res.first != RC::SUCCESS) {
      break;
    }
  }

  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_cols(std::vector<std::string> &cols)
{
  while (true) {
    auto [rc, tk] = lexer_.next_if(Token::ID_T);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    cols.emplace_back(std::move(lexer_.template cur_ref<std::string>()));
    auto [rc1, tk1] = lexer_.next_if(Token::COMMAS_T);
    if (rc1 != RC::SUCCESS) {
      break;
    }
  }

  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_values(std::vector<Value> &values)
{
  while (true) {
    Value v;
    auto rc = parse_value(v);
    if (!rc_success(rc)) {
      return rc;
    }
    if (v.type_ == AttrType::REL_ATTR || v.type_ == AttrType::UNDEFINED) {
      return RC::SYNTAX_ERROR;
    }
    values.emplace_back(std::move(v));
    auto [rc1, tk1] = lexer_.next_if(Token::COMMAS_T);
    if (rc1 != RC::SUCCESS) {
      break;
    }
  }
  return RC::SUCCESS;
}

// Tables
template <typename InputType>
RC Parser<InputType>::parse_from(std::vector<std::string> &from_list)
{
  // From Token first
  auto [rc, tk] = lexer_.next();
  if (rc != RC::SUCCESS || tk != Token::FROM_T) {
    return RC::SYNTAX_ERROR;
  }
  while (true) {
    auto [rc1, tk1] = lexer_.next_if(Token::ID_T);
    if (rc1 != RC::SUCCESS) {
      return rc1;
    }
    // LOG_DEBUG << lexer_.template cur_ref<std::string>();
    from_list.push_back(std::move(lexer_.template cur_ref<std::string>()));
    auto [rc2, tk2] = lexer_.next_if(Token::COMMAS_T);
    if (rc2 != RC::SUCCESS) {
      break;
    }
  }

  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_conds(std::vector<Condition> &cond_list)
{
  auto [rc, tk] = lexer_.next();
  if (rc != RC::SUCCESS) {
    return rc;
  }

  if (tk != Token::WHERE_T) {
    rc = tk == Token::COLON_T ? RC::SUCCESS : RC::SYNTAX_ERROR;
    return rc;
  }

  while (true) {
    Condition c;

    // 先解析左边的value
    if (rc = parse_value(c.left_); rc != RC::SUCCESS) {
      return rc;
    }

    // 解析符号
    auto [rc1, tk1] = lexer_.next();
    if (rc1 != RC::SUCCESS) {
      return rc1;
    }
    auto op = token2op(tk1);
    if (op == CondOp::UNDEFINED) {
      return RC::SYNTAX_ERROR;
    }
    c.op_ = op;

    // 解析右边的value
    if (rc = parse_value(c.right_); rc != RC::SUCCESS) {
      return rc;
    }

    cond_list.emplace_back(std::move(c));

    // 如果下一个token不是 ‘，’， 则解析完了
    auto [rc2, tk2] = lexer_.next_if(Token::COMMAS_T);
    if (rc2 != RC::SUCCESS) {
      break;
    }
  }
  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_value(Value &v)
{
  auto [rc, tk] = lexer_.next();
  if (rc != RC::SUCCESS) {
    return rc;
  }

  if (tk == Token::ID_T) {
    v.value_ = RelAttr{};
    auto &&attr = v.get<RelAttr &>();
    v.type_ = AttrType::REL_ATTR;
    attr.table_ = std::move(lexer_.template cur_ref<std::string>());
    auto [rc1, tk1] = lexer_.next_if(Token::DOT_T);
    if (rc1 == RC::SUCCESS) {
      auto [rc2, tk2] = lexer_.next_if(Token::ID_T);
      if (rc2 != RC::SUCCESS) {
        return rc2;
      }
      attr.attribute_ = std::move(lexer_.template cur_ref<std::string>());
    } else {
      attr.attribute_ = std::move(attr.table_);
    }
    LOG_DEBUG << std::any_cast<RelAttr>(v.value_).table_ << "." << std::any_cast<RelAttr>(v.value_).attribute_;
  } else {
    switch (tk) {
      case Token::FLOAT_T:
        v.value_ = lexer_.template cur_ref<float>();
        v.type_ = AttrType::FLOATS;
        break;
      case Token::INTEGER_T:
        v.value_ = lexer_.template cur_ref<long>();
        v.type_ = AttrType::INTS;
        break;
      case Token::STRING_T:
        v.value_ = std::move(lexer_.cur_any_ref());
        v.type_ = AttrType::STRING;
        break;
      case Token::NULL_T:
        v.type_ = AttrType::NULL_A;
        break;
      default:
        assert(false);
    }
  }

  return RC::SUCCESS;
}

// DELETE FROM 表名称 WHERE 列名称 = 值
// DELETE FROM 表
// DELETE * FRRM 表
template <typename InputType>
RC Parser<InputType>::parse_delete()
{
  auto [rc, _] = lexer_.next_if(Token::DELETE_T);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  DeleteAst d;

  auto [rc2, tk2] = lexer_.next_if(Token::STAR_T);
  if (rc = parse_from(d.tables_); rc != RC::SUCCESS) {
    return rc;
  }

  auto [rc3, tk3] = lexer_.peek();

  // 我这里规定这种文法错误: DELETE * FROM t1 WHERE name='pink';
  if (tk3 == Token::WHERE_T && tk2 == Token::STAR_T) {
    return RC::SYNTAX_ERROR;
  }

  if (rc = parse_conds(d.conds_); rc != RC::SUCCESS) {
    return rc;
  }

  auto [rc4, tk4] = lexer_.next_if(Token::COLON_T);
  query_ = std::move(d);
  return rc4;
}

// INSERT INTO 表名称 VALUES (值1, 值2,....)
// INSERT INTO table_name (列1, 列2,...) VALUES (值1, 值2,....)
template <typename InputType>
RC Parser<InputType>::parse_insert()
{
  InsertAst insert;
  if (lexer_.next_if(Token::INSERT_T).first != RC::SUCCESS) {
    LOG_DEBUG << "it is not insert query";
    return RC::SYNTAX_ERROR;
  }
  if (lexer_.next_if(Token::INTO_T).first != RC::SUCCESS) {
    LOG_DEBUG << "'INTO' not exist";
    return RC::SYNTAX_ERROR;
  }
  // table name
  if (lexer_.next_if(Token::ID_T).first != RC::SUCCESS) {
    LOG_DEBUG << "it is not a id";
    return RC::SYNTAX_ERROR;
  }

  insert.table_name_ = std::move(lexer_.template cur_ref<std::string>());
  auto [rc, tk] = lexer_.next();
  if (rc != RC::SUCCESS) {
    return rc;
  }
  if (tk == Token::LBRACE_T) {
    rc = parse_cols(insert.cols_);
    if (rc != RC::SUCCESS) {
      LOG_DEBUG << "parse cols fail";
      return rc;
    }
    if (lexer_.next_if(Token::RBRACE_T).first != RC::SUCCESS) {
      LOG_DEBUG << "miss rbrace";
      return RC::SYNTAX_ERROR;
    }
  } else if (tk == Token::VALUES_T) {
    // DO NOTHING
  } else {
    LOG_DEBUG << "wrong token";
    return RC::SYNTAX_ERROR;
  }

  if (tk != Token::VALUES_T) {
    auto [rc1, tk1] = lexer_.next_if(Token::VALUES_T);
    if (rc1 != RC::SUCCESS) {
      LOG_DEBUG << "miss VALUES";
      return rc1;
    }
  }

  // 最后三个符号 ')' ','  ')'
  auto [rc1, tk1] = lexer_.next_if(Token::LBRACE_T);
  if (rc1 != RC::SUCCESS) {
    LOG_DEBUG << "miss lbrace";
    return rc1;
  }
  if (parse_values(insert.values_) != RC::SUCCESS) {
    LOG_DEBUG << "parse values fail";
    return RC::SYNTAX_ERROR;
  }
  if (lexer_.next_if(Token::RBRACE_T).first != RC::SUCCESS) {
    LOG_DEBUG << "miss rbrace";
    return RC::SYNTAX_ERROR;
  }
  if (lexer_.next_if(Token::COLON_T).first != RC::SUCCESS) {
    LOG_DEBUG << "miss colon";
    return RC::SYNTAX_ERROR;
  }

  query_ = std::move(insert);
  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_create()
{
  if (!rc_success(lexer_.next_if(Token::CREATE_T).first)) {
    return RC::SYNTAX_ERROR;
  }
  auto [rc, tk] = lexer_.peek();
  if (!rc_success(RC::SUCCESS)) {
    return RC::SYNTAX_ERROR;
  }

  switch (tk) {
    case Token::TABLE_T:
      return parse_create_table();
    // TODO index
    default:
      return RC::SYNTAX_ERROR;
  }
  // make compiler happy
  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_create_table()
{
  if (!rc_success(lexer_.next_if(Token::TABLE_T).first)) {
    LOG_DEBUG << "miss 'TABLE'";
    return RC::SYNTAX_ERROR;
  }
  CreateTableAst create_table;
  if (!rc_success(lexer_.next_if(Token::ID_T).first)) {
    LOG_DEBUG << "miss table's name";
    return RC::SYNTAX_ERROR;
  }
  create_table.table_name_ = std::move(lexer_.template cur_ref<std::string>());
  if (!rc_success(lexer_.next_if(Token::LBRACE_T).first)) {
    LOG_DEBUG << "miss '('";
    return RC::SYNTAX_ERROR;
  }
  if (!rc_success(parse_col_attrs(create_table.col_attrs_))) {
    LOG_DEBUG << "parse colums' attributes fail";
    return RC::SYNTAX_ERROR;
  }
  if (!rc_success(lexer_.next_if(Token::RBRACE_T).first)) {
    LOG_DEBUG << "miss ')'";
    return RC::SYNTAX_ERROR;
  }
  if (!rc_success(lexer_.next_if(Token::COLON_T).first)) {
    LOG_DEBUG << "miss ';'";
    return RC::SYNTAX_ERROR;
  }
  query_ = create_table;
  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_col_attrs(std::vector<ColAttr> &col_attrs)
{
  while (true) {
    if (!rc_success(lexer_.next_if(Token::ID_T).first)) {
      LOG_DEBUG << "expect Token::ID_T";
      return RC::SYNTAX_ERROR;
    }
    ColAttr c;
    c.name_ = std::move(lexer_.template cur_ref<std::string>());
    if (!rc_success(parse_col_data_type(c))) {
      LOG_DEBUG << "parse colum's datatype fail";
      return RC::SYNTAX_ERROR;
    }
    col_attrs.emplace_back(std::move(c));
    if (!rc_success(lexer_.next_if(Token::COMMAS_T).first)) {
      break;
    }
  }

  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_col_data_type(ColAttr &c)
{
  auto [rc, tk] = lexer_.next();
  if (!rc_success(rc)) {
    return RC::SYNTAX_ERROR;
  }
  switch (tk) {
    case Token::S_INT_T:
      c.type_ = "INT";
      break;
    case Token::S_FLOAT_T:
      c.type_ = "FLOAT";
      break;
    case Token::S_CHAR_T:
    case Token::S_VARCHAR_T: {
      c.type_ = tk == Token::S_CHAR_T ? "CHAR" : "VARCHAR";
      if (!rc_success(lexer_.next_if(Token::LBRACE_T).first)) {
        LOG_DEBUG << "miss '('";
        return RC::SYNTAX_ERROR;
      }
      if (!rc_success(lexer_.next_if(Token::INTEGER_T).first)) {
        LOG_DEBUG << "miss text attribute's size";
        return RC::SYNTAX_ERROR;
      }
      c.size_ = static_cast<size_t>(lexer_.template cur_ref<long>());
      if (!rc_success(lexer_.next_if(Token::RBRACE_T).first)) {
        LOG_DEBUG << "miss ')'";
        return RC::SYNTAX_ERROR;
      }
    } break;
    default: {
      LOG_DEBUG << "wrong Token";
      return RC::SYNTAX_ERROR;
    }
  }

  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_drop()
{
  DropAst ds;
  {
    auto [rc, tk] = lexer_.next();
    if (!rc_success(rc) || tk != Token::DROP_T) {
      return rc;
    }
  }

  {
    auto [rc, tk] = lexer_.next();
    if (!rc_success(rc) || tk != Token::TABLE_T) {
      return rc;
    }
  }

  {
    auto [rc, tk] = lexer_.next();
    if (!rc_success(rc) || tk != Token::ID_T) {
      return rc;
    }
    ds.table_ = std::move(std::any_cast<std::string&>(lexer_.cur_any_ref()));
  }

  query_ = std::move(ds);

  {
    auto [rc, tk] = lexer_.next();
    if (rc_success(rc) && tk == Token::COLON_T) {
      return RC::SUCCESS;
    }
  }

  return RC::SYNTAX_ERROR;
}

/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-06 16:25:58
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-08 13:58:11
 * @FilePath: /tadis/src/sql/parser/parser.hpp
 * @Description: 词法解析
 */
#pragma once

#include <common/logger.hpp>
#include <sql/parser/lex.hpp>
#include <sql/parser/parser_define.hpp>
#include <boost/noncopyable.hpp>
#include <variant>

template <typename InputType>
class Parser : public boost::noncopyable {
public:
  Parser(InputType input) : lexer_(input)
  {}

  RC parse();

  const Query &query()
  {
    return query_;
  }

private:
  // select
  RC parse_select();
  RC parse_select_attrs(Select &s);
  RC parse_select_from(Select &s);
  RC parse_value(Value &v);
  RC parse_select_conds(Select &s);

private:
  Lexer<InputType> lexer_;
  Query query_;
};

template <typename InputType>
RC Parser<InputType>::parse()
{
  auto [rc, tk] = lexer_.peek();
  switch (tk) {
    case Token::SELECT_T: {
      return parse_select();
    }
    // case Token::CREATE_T: {

    // }
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

  query_ = Select{};
  auto &&select = std::get<Select>(query_);
  if (rc = parse_select_attrs(select); rc != RC::SUCCESS) {
    LOG_DEBUG << "FAIL";
    return rc;
  }
  if (rc = parse_select_from(select); rc != RC::SUCCESS) {
    LOG_DEBUG << "FAIL";
    return rc;
  }
  if (rc = parse_select_conds(select); rc != RC::SUCCESS) {
    LOG_DEBUG << "FAIL";
    return rc;
  }

  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_select_attrs(Select &s)
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
      if (s.selist_.size() > 0) {
        return RC::SYNTAX_ERROR;
      }
      s.selist_.emplace_back(std::move(rel));
      return rc;
    }

    if (tk != Token::ID_T) {
      LOG_DEBUG << "tk isn't ID_T";
      return RC::SYNTAX_ERROR;
    }

    RelAttr rel_attr;
    std::string name;
    name = std::move(std::any_cast<std::string>(lexer_.cur_val_ref()));

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
      rel_attr.attribute_ = std::move(std::any_cast<std::string>(lexer_.cur_val_ref()));
    } else {
      rel_attr.attribute_ = std::move(name);
    }

    LOG_DEBUG << "RelAttr " << rel_attr.table_ << "." << rel_attr.attribute_;

    s.selist_.emplace_back(std::move(rel_attr));

    auto next_res = lexer_.next_if(Token::COMMAS_T);
    if (next_res.first != RC::SUCCESS) {
      break;
    }
  }

  return RC::SUCCESS;
}

// Tables
template <typename InputType>
RC Parser<InputType>::parse_select_from(Select &s)
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
    LOG_DEBUG << std::any_cast<std::string &>(lexer_.cur_val_ref());
    s.from_list_.push_back(std::move(std::any_cast<std::string &>(lexer_.cur_val_ref())));
    auto [rc2, tk2] = lexer_.next_if(Token::COMMAS_T);
    if (rc2 != RC::SUCCESS) {
      break;
    }
  }

  return RC::SUCCESS;
}

template <typename InputType>
RC Parser<InputType>::parse_select_conds(Select &s)
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

    if (rc = parse_value(c.left_); rc != RC::SUCCESS) {
      return rc;
    }

    auto [rc1, tk1] = lexer_.next();
    if (rc1 != RC::SUCCESS) {
      return rc1;
    }

    auto op = token2op(tk1);
    if (op == CondOp::UNDEFINED) {
      return RC::SYNTAX_ERROR;
    }
    c.op_ = op;

    if (rc = parse_value(c.right_); rc != RC::SUCCESS) {
      return rc;
    }

    s.cond_list_.emplace_back(std::move(c));

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
    attr.table_ = std::move(std::any_cast<std::string>(lexer_.cur_val_ref()));
    auto [rc1, tk1] = lexer_.next_if(Token::DOT_T);
    if (rc1 == RC::SUCCESS) {
      auto [rc2, tk2] = lexer_.next_if(Token::ID_T);
      if (rc2 != RC::SUCCESS) {
        return rc2;
      }
      attr.attribute_ = std::move(std::any_cast<std::string>(lexer_.cur_val_ref()));
    }
    LOG_DEBUG << std::any_cast<RelAttr>(v.value_).table_ << "." << std::any_cast<RelAttr>(v.value_).attribute_;
  } else {
    switch (tk) {
      case Token::FLOAT_T:
        v.value_ = std::any_cast<float>(lexer_.cur_val_ref());
        v.type_ = AttrType::FLOATS;
        break;
      case Token::INTEGER_T:
        v.value_ = std::any_cast<long>(lexer_.cur_val_ref());
        v.type_ = AttrType::INTS;
        break;
      case Token::STRING_T:
        v.value_ = std::move(lexer_.cur_val_ref());
        v.type_ = AttrType::STRING;
        break;
      case Token::NULL_T:
        v.type_ = AttrType::NULL_A;
        break;
    }
  }

  return RC::SUCCESS;
}

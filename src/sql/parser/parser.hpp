/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-06 16:25:58
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-06 20:15:20
 * @FilePath: /tadis/src/sql/parser/parser.hpp
 * @Description: 词法解析
 */
#pragma once

#include <common/logger.hpp>
#include <sql/parser/lex.hpp>
#include <sql/sql_define.hpp>
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
  RC parse_select_froms(Select &s);
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
      LOG_DEBUG << "Parse select";
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
    return rc;
  }
  if (rc = parse_select_froms(select); rc != RC::SUCCESS) {
    return rc;
  }
  if (rc = parse_select_conds(select); rc != RC::SUCCESS) {
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
    if (tk == Token::STAR_T) {
      LOG_DEBUG << "parse START_T";
      RelAttr rel;
      rel.attribute_ = "*";
      s.selist_.emplace_back(std::move(rel));
      return rc;
    }
    if (tk != Token::ID_T) {
      LOG_DEBUG << "parse ID_T";
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

    LOG_DEBUG << "Get RelAttr " << rel_attr.table_ << "." << rel_attr.attribute_;

    s.selist_.emplace_back(std::move(rel_attr));

    auto next_res = lexer_.next_if(Token::COMMAS_T);
    if (next_res.first != RC::SUCCESS) {
      break;
    }
  }
}

template <typename InputType>
RC Parser<InputType>::parse_select_froms(Select &s)
{
  return RC::SYNTAX_ERROR;
}

template <typename InputType>
RC Parser<InputType>::parse_select_conds(Select &s)
{
  return RC::SYNTAX_ERROR;
}

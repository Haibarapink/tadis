/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-06 16:25:58
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-06 16:55:21
 * @FilePath: /tadis/src/sql/parser/parser.hpp
 * @Description: 词法解析
 */
#pragma once

#include <common/logger.hpp>
#include <sql/parser/lex.hpp>
#include <sql/sql_define.hpp>
#include <boost/noncopyable.hpp>

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
  RC parse_select_attrs();
  RC parse_select_froms();
  RC parse_select_conds();

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
  return RC::SYNTAX_ERROR;
}

template <typename InputType>
RC Parser<InputType>::parse_select_attrs()
{
  return RC::SYNTAX_ERROR;
}

template <typename InputType>
RC Parser<InputType>::parse_select_froms()
{
  return RC::SYNTAX_ERROR;
}

template <typename InputType>
RC Parser<InputType>::parse_select_conds()
{
  return RC::SYNTAX_ERROR;
}

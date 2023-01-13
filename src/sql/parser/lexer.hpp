/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-05 19:39:35
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-13 18:34:54
 * @FilePath: /tadis/src/common/logger.hpp
 * @Description: 词法分析
 */
#pragma once
#include "common/rc.hpp"
#include <any>
#include <boost/core/noncopyable.hpp>
#include <cassert>
#include <tuple>
#include <string>
#include <string_view>
#include <boost/noncopyable.hpp>

template <typename T>
using LexResult = std::pair<RC, T>;

enum class Token {
  SELECT_T,
  INSERT_T,
  CREATE_T,
  DELETE_T,
  UPDATE_T,
  FROM_T,
  WHERE_T,
  INTO_T,
  VALUES_T,
  TABLE_T,
  ID_T,

  PLUS_T,
  MINUS_T,
  FLOAT_T,
  INTEGER_T,
  STRING_T,

  COLON_T,
  DOT_T,     // '.'
  COMMAS_T,  // ','
  STAR_T,    // '*'
  ASSIGN_T,  // ‘=’
  NOT_EQ_T,
  GREATER_T,
  SMALLER_T,
  LBRACE_T,
  RBRACE_T,

  IN_T,  // in

  IS_T,
  NOT_T,
  NULL_T,
  NULLABLE_T,

  // sql datatypes
  //    TEXT
  S_CHAR_T,
  S_VARCHAR_T,

  // NUMBER
  S_INT_T,
  S_FLOAT_T,
  // S_DOUBLE_T,

  // TODO date

  //....

  BAD_EXPR  // fail!
};

template <typename InputType>
class Lexer : public boost::noncopyable {
public:
  Lexer()
  {}
  Lexer(InputType input)
  {
    init(std::move(input));
  }
  ~Lexer()
  {}

  void init(InputType input)
  {
    input_ = std::move(input);
    p_ = 0;

    spec_token_.emplace("SELECT", Token::SELECT_T);
    spec_token_.emplace("INSERT", Token::INSERT_T);
    spec_token_.emplace("UPDATE", Token::UPDATE_T);
    spec_token_.emplace("CREATE", Token::CREATE_T);
    spec_token_.emplace("DELETE", Token::DELETE_T);

    spec_token_.emplace("INTO", Token::INTO_T);
    spec_token_.emplace("FROM", Token::FROM_T);
    spec_token_.emplace("WHERE", Token::WHERE_T);
    spec_token_.emplace("INFO", Token::INTO_T);
    spec_token_.emplace("TABLE", Token::TABLE_T);
    spec_token_.emplace("VALUES", Token::VALUES_T);

    spec_token_.emplace("IN", Token::IN_T);
    spec_token_.emplace("IS", Token::IS_T);
    spec_token_.emplace("NOT", Token::NOT_T);
    spec_token_.emplace("NULL", Token::NULL_T);
    spec_token_.emplace("NULLABLE", Token::NULLABLE_T);

    // For sql data types
    spec_token_.emplace("CHAR", Token::S_CHAR_T);
    spec_token_.emplace("VARCHAR", Token::S_VARCHAR_T);
    spec_token_.emplace("INT", Token::S_INT_T);
    spec_token_.emplace("FLOAT", Token::S_FLOAT_T);
  }

  LexResult<Token> next();

  // 如果不一样 则返回前一个token,并且返回 RC::SYNTAX_ERROR和下一个 Token
  LexResult<Token> next_if(Token t);

  // peek 下一个 token，不会前进。
  LexResult<Token> peek();

  std::any &cur_any_ref()
  {
    return cur_val_;
  }

  template <typename T>
  T &cur_ref()
  {
    return std::any_cast<T &>(cur_val_);
  }

  std::tuple<RC, Token, size_t> internal_next();

private:
  void skip_blank()
  {
    for (; p_ < input_.size() && input_[p_] == ' '; ++p_) {}
  }

  void to_upper(std::string &s)
  {
    for (auto &ch : s) {
      if (ch >= 'a' && ch <= 'z') {
        ch = ch - 'a' + 'A';
      }
    }
  }

  Token parse_num()
  {
    Token token = Token::BAD_EXPR;
    long num = 0;
    size_t end = p_;
    for (; input_[end] >= '0' && input_[end] <= '9'; end++) {
      num = input_[end] - '0' + num * 10;
    }

    // float
    if (input_[end] == '.') {
      size_t old_end = end;
      end++;
      for (; input_[end] >= '0' && input_[end] <= '9'; end++) {}

      if (old_end == end - 1) {
        p_ = end;
        return Token::BAD_EXPR;
      }

      std::string float_str{input_.data() + p_, end - p_ - 1};
      cur_val_ = std::stof(float_str);
      token = Token::FLOAT_T;
      p_ = end;
      return token;
    }

    p_ = end;
    cur_val_ = num;
    token = Token::INTEGER_T;
    return token;
  }

  std::any cur_val_;
  InputType input_;
  size_t p_{0};

  std::map<std::string, Token> spec_token_;
};

template <typename Input>
LexResult<Token> Lexer<Input>::next_if(Token t)
{
  auto [rc, tk, last_p] = internal_next();
  if (tk != t) {
    p_ = last_p;
    rc = RC::SYNTAX_ERROR;
  } else {
    rc = RC::SUCCESS;
  }
  return {rc, tk};
}

template <typename Input>
LexResult<Token> Lexer<Input>::next()
{
  auto [rc, tk, last_p] = internal_next();
  return {rc, tk};
}

template <typename Input>
LexResult<Token> Lexer<Input>::peek()
{
  auto [rc, tk, last_p] = internal_next();
  p_ = last_p;
  return {rc, tk};
}

template <typename Input>
std::tuple<RC, Token, size_t> Lexer<Input>::internal_next()
{
  RC rc = RC::SYNTAX_ERROR;
  Token token = Token::BAD_EXPR;
  size_t last_p = p_;
  skip_blank();
  char ch = input_[p_];
  while (p_ < input_.size()) {
    switch (ch) {
      case 'a' ... 'z':
      case 'A' ... 'Z':
      case '_': {
        size_t end = p_;
        for (; (input_[end] >= 'a' && input_[end] <= 'z') || (input_[end] >= 'A' && input_[end] <= 'Z') ||
               input_[end] == '_' || (input_[end] >= '0' && input_[end] <= '9');
             end++) {}
        rc = RC::SUCCESS;
        auto id = std::string{input_.data() + p_, end - p_};
        auto cp_id = id;
        to_upper(id);
        token = Token::ID_T;
        if (auto iter = spec_token_.find(id); iter != spec_token_.end()) {
          token = iter->second;
        }

        if (token == Token::ID_T) {
          cur_val_ = std::string{std::move(cp_id)};
        }

        p_ = end;
        return {rc, token, last_p};
      }

      case '0': {
        // 0...
        if (p_ + 1 < input_.size() && (input_[p_ + 1] >= '0' && input_[p_ + 1] <= '9')) {
          return {RC::SYNTAX_ERROR, Token::BAD_EXPR, last_p};
        }

        token = parse_num();
        rc = token == Token::BAD_EXPR ? RC::SYNTAX_ERROR : RC::SUCCESS;
        return {rc, token, last_p};
      }

      case '1' ... '9': {
        token = parse_num();
        rc = token == Token::BAD_EXPR ? RC::SYNTAX_ERROR : RC::SUCCESS;
        return {rc, token, last_p};
      }

      case '+': {
        p_++;
        return {RC::SUCCESS, Token::PLUS_T, last_p};
      }

      case '-': {
        if (p_ + 1 < input_.size() && input_[p_ + 1] >= '0' && input_[p_ + 1] <= '9') {
          p_++;
          auto token = parse_num();
          if (token == Token::INTEGER_T) {
            cur_val_ = std::any_cast<long>(cur_val_) * -1;
          } else if (token == Token::FLOAT_T) {
            cur_val_ = std::any_cast<float>(cur_val_) * -1;
          }
          return {RC::SUCCESS, token, last_p};
        }
        p_++;
        return {RC::SUCCESS, Token::MINUS_T, last_p};
      }

      case '=': {
        p_++;
        return {RC::SUCCESS, Token::ASSIGN_T, last_p};
      }

      case '>': {
        p_++;
        return {RC::SUCCESS, Token::GREATER_T, last_p};
      }

      case '<': {
        if (p_ + 1 < input_.size() && input_[p_ + 1] == '>') {
          p_ += 2;
          return {RC::SUCCESS, Token::NOT_EQ_T, last_p};
        }
        p_++;
        return {RC::SUCCESS, Token::SMALLER_T, last_p};
      }

      case ',': {
        p_++;
        return {RC::SUCCESS, Token::COMMAS_T, last_p};
      }

      case '*': {
        p_++;
        return {RC::SUCCESS, Token::STAR_T, last_p};
      }

      case '.': {
        p_++;
        return {RC::SUCCESS, Token::DOT_T, last_p};
      }

      case ';': {
        p_++;
        return {RC::SUCCESS, Token::COLON_T, last_p};
      }

      case '(': {
        p_++;
        return {RC::SUCCESS, Token::LBRACE_T, last_p};
      }

      case ')': {
        p_++;
        return {RC::SUCCESS, Token::RBRACE_T, last_p};
      }

      case '\'': {
        auto end = p_ + 1;
        for (; end < input_.size() && input_[end] != '\''; ++end) {}
        if (end == p_ + 1) {
          return {RC::SYNTAX_ERROR, token, last_p};
        }
        cur_val_ = std::string{input_.data() + p_ + 1, end - p_ - 1};
        p_ = end + 1;

        return {RC::SUCCESS, Token::STRING_T, last_p};
      }

      default: {
        return {rc, token, last_p};
      }
    }
  }
  // unuse code
  return {rc, token, last_p};
}
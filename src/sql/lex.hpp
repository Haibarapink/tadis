#pragma once
#include "common/code.hpp"
#include <any>
#include <boost/core/noncopyable.hpp>
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

    spec_token_.emplace("FROM", Token::FROM_T);
    spec_token_.emplace("WHERE", Token::WHERE_T);
    spec_token_.emplace("INFO", Token::INTO_T);
    spec_token_.emplace("TABLE", Token::TABLE_T);
    spec_token_.emplace("VALUES", Token::VALUES_T);
  }

  // 如果下一个是期待的token 则返回， 否则就不动。
  LexResult<Token> next_expect(Token token);
  LexResult<Token> next();

  std::any &cur_val_ref()
  {
    return cur_val_;
  }
  std::tuple<RC, Token, size_t> internal_next();

private:
  void skip_blank()
  {
    for (; p_ < input_.size() && input_[p_] != ' '; ++p_) {}
  }

  void to_upper(std::string &s)
  {
    for (auto &ch : s) {
      if (ch >= 'a' && ch <= 'z') {
        ch = ch - 'a' + 'A';
      }
    }
  }

  std::any cur_val_;
  InputType input_;
  size_t p_{0};

  std::map<std::string, Token> spec_token_;
};

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
        to_upper(id);
        token = Token::ID_T;
        if (auto iter = spec_token_.find(id); iter != spec_token_.end()) {
          token = iter->second;
        }

        if (token == Token::ID_T) {
          cur_val_ = std::string{std::move(id)};
        }

        return {rc, token, last_p};
      }

      case '+': {
        p_++;
        return {RC::SUCCESS, Token::PLUS_T, last_p};
      }

      case '=': {
        p_++;
        return {RC::SUCCESS, Token::ASSIGN_T, last_p};
      }

      default: {
        return {rc, token, last_p};
      }
    }
  }

  return {rc, token, last_p};
}
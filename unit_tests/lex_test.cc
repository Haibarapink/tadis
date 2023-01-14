#include <boost/core/lightweight_test.hpp>
#include <string>
#include <vector>
#include <sql/parser/lexer.hpp>

// void basic_lexer_unit_test()
// {
//   Lexer<std::string> lexer;
//   std::vector<std::string> words = {"hello", "-0.1", "world", "4321", "0.996", "000", "012"};
//   std::string input;
//   for (auto &i : words) {
//     input.append(i);
//     input.append(",");
//   }

//   auto commas_test = [&] {
//     auto [r, t, p] = lexer.internal_next();
//     BOOST_TEST(r == RC::SUCCESS);
//     BOOST_TEST(t == Token::COMMAS_T);
//     p++;
//   };

//   size_t p = 0;
//   {
//     auto [rc, token, last_p] = lexer.internal_next();
//     BOOST_TEST(rc == RC::SUCCESS);
//     BOOST_TEST(token == Token::ID_T);
//     BOOST_TEST(std::any_cast<std::string>(lexer.cur_val_ref()) == "hello");
//     commas_test();
//   }

//   {
//     auto [rc, token, last_p] = lexer.internal_next();
//     BOOST_TEST(rc == RC::SUCCESS);
//     BOOST_TEST(token == Token::FLOAT_T);
//     BOOST_TEST(std::any_cast<float>(lexer.cur_val_ref()) == -0.1);
//     commas_test();
//   }

//   {
//     auto [rc, token, last_p] = lexer.internal_next();
//     BOOST_TEST(rc == RC::SUCCESS);
//     BOOST_TEST(token == Token::ID_T);
//     BOOST_TEST(std::any_cast<std::string>(lexer.cur_val_ref()) == "world");
//     commas_test();
//   }
// }

void test_simple_select()
{
  Lexer<std::string> lexer;
  std::string s1 = "select id, name, age from student where id=1";
  lexer.init(std::move(s1));

  // test select token
  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::SELECT_T);
    BOOST_TEST_EQ(last_p, 0);
  }

  // test id_t
  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::ID_T);
    BOOST_TEST_EQ(last_p, 6);
    BOOST_TEST_EQ(std::any_cast<std::string>(lexer.cur_any_ref()), std::string("id"));
  }

  // test dot_t
  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::COMMAS_T);
  }

  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::ID_T);
    BOOST_TEST(std::any_cast<std::string>(lexer.cur_any_ref()) == std::string{"name"});
  }

  for (auto i = 0; i < 5; ++i) {
    lexer.internal_next();
  }

  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::ID_T);
    BOOST_TEST(std::any_cast<std::string>(lexer.cur_any_ref()) == std::string{"id"});
  }

  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::ASSIGN_T);
  }

  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::INTEGER_T);
    BOOST_TEST(std::any_cast<long>(lexer.cur_any_ref()) == 1);
  }
}

void test_nexts()
{
  std::string s1 = "select id, name, age from student where num=-0.0123";
  Lexer<std::string_view> lexer{std::string_view(s1.data(), s1.size())};
  BOOST_TEST(lexer.next_if(Token::SELECT_T).first == RC::SUCCESS);
  BOOST_TEST(lexer.next_if(Token::ID_T).first == RC::SUCCESS);
  BOOST_TEST(lexer.next_if(Token::STAR_T).first == RC::SYNTAX_ERROR);
  BOOST_TEST(lexer.next_if(Token::COMMAS_T).first == RC::SUCCESS);
  BOOST_TEST(lexer.next_if(Token::DOT_T).first == RC::SYNTAX_ERROR);
  BOOST_TEST(lexer.next_if(Token::ID_T).first == RC::SUCCESS);
}

void test_nexts2()
{
  std::string s2 = "0.01=-100,<>";
  Lexer<std::string_view> lexer{std::string_view(s2.data(), s2.size())};
  BOOST_TEST(lexer.next_if(Token::INTEGER_T).first == RC::SYNTAX_ERROR);
  BOOST_TEST(lexer.next_if(Token::FLOAT_T).first == RC::SUCCESS);
  BOOST_TEST(lexer.next_if(Token::ASSIGN_T).first == RC::SUCCESS);
  BOOST_TEST(lexer.next_if(Token::INTEGER_T).first == RC::SUCCESS);
  BOOST_TEST(std::any_cast<long>(lexer.cur_any_ref()) == -100);
  BOOST_TEST(lexer.next_if(Token::COMMAS_T).first == RC::SUCCESS);
  BOOST_TEST(lexer.next_if(Token::NOT_EQ_T).first == RC::SUCCESS);
}

void test_string()
{
  std::string s = " \'Hello\'123+\'World\'";
  Lexer<std::string> lexer{s};
  BOOST_TEST(lexer.next_if(Token::STRING_T).first == RC::SUCCESS);
  BOOST_TEST(std::any_cast<std::string>(lexer.cur_any_ref()) == "Hello");
  BOOST_TEST(lexer.next_if(Token::INTEGER_T).first == RC::SUCCESS);
  BOOST_TEST(lexer.next_if(Token::PLUS_T).first == RC::SUCCESS);
  BOOST_TEST(lexer.next_if(Token::STRING_T).first == RC::SUCCESS);
  BOOST_TEST(std::any_cast<std::string>(lexer.cur_any_ref()) == "World");
}

int main(int argc, char *argv[])
{
  test_simple_select();
  // basic_lexer_unit_test();
  test_nexts();
  test_nexts2();
  test_string();
  return boost::report_errors();
}
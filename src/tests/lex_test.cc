#include <boost/core/lightweight_test.hpp>
#include <string>
#include <vector>
#include <sql/lex.hpp>

void basic_lexer_unit_test()
{
  Lexer<std::string> lexer;
  std::vector<std::string> words = {"hello", "-0.1", "world", "4321", "0.996", "000", "012"};
  std::string input;
  for (auto &i : words) {
    input.append(i);
    input.append(",");
  }

  auto commas_test = [&] {
    auto [r, t, p] = lexer.internal_next();
    BOOST_TEST(r == RC::SUCCESS);
    BOOST_TEST(t == Token::COMMAS_T);
    p++;
  };

  size_t p = 0;
  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::ID_T);
    BOOST_TEST(std::any_cast<std::string>(lexer.cur_val_ref()) == "hello");
    commas_test();
  }

  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::FLOAT_T);
    BOOST_TEST(std::any_cast<float>(lexer.cur_val_ref()) == -0.1);
    commas_test();
  }

  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::ID_T);
    BOOST_TEST(std::any_cast<std::string>(lexer.cur_val_ref()) == "world");
    commas_test();
  }
}

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
    BOOST_TEST_EQ(std::any_cast<std::string>(lexer.cur_val_ref()), std::string("id"));
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
    BOOST_TEST(std::any_cast<std::string>(lexer.cur_val_ref()) == std::string{"name"});
  }

  for (auto i = 0; i < 5; ++i) {
    lexer.internal_next();
  }

  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::ID_T);
    BOOST_TEST(std::any_cast<std::string>(lexer.cur_val_ref()) == std::string{"id"});
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
    BOOST_TEST(std::any_cast<long>(lexer.cur_val_ref()) == 1);
  }
}

int main(int argc, char *argv[])
{
  test_simple_select();
  return boost::report_errors();
}
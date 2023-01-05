#include <sql/lex.hpp>
#include <boost/core/lightweight_test.hpp>

void test1()
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
    BOOST_TEST_EQ(std::any_cast<std::string>(lexer.cur_val_ref()), std::string("SELECT"));
  }

  // test dot_t
  {
    auto [rc, token, last_p] = lexer.internal_next();
    BOOST_TEST(rc == RC::SUCCESS);
    BOOST_TEST(token == Token::DOT_T);
  }
}

int main(int argc, char *argv[])
{
  test1();
}
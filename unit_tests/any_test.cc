/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-08 13:03:22
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-14 23:21:02
 * @FilePath: /tadis/src/tests/any_test.cc
 * @Description: any usage example
 */
#include <any>
#include <boost/core/lightweight_test.hpp>

void test()
{
  std::any a = std::string("hello");
  std::any_cast<std::string &>(a).append("hello");
  //   BOOST_TEST(std::any_cast<std::string>(a) == "hello");
  BOOST_TEST(std::any_cast<std::string>(a) == "hellohello");
}

int main(int argc, char *argv[])
{
  test();
  return boost::report_errors();
}

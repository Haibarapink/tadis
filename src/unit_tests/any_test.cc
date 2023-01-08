/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-08 13:03:22
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-08 13:11:51
 * @FilePath: /tadis/src/tests/any_test.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 * https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AEint
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

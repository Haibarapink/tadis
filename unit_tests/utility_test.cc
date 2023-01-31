/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-15 15:22:24
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-16 10:51:54
 * @FilePath: /tadis/unit_tests/utility_test.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 * https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "common/utility.hpp"
#include <boost/core/lightweight_test.hpp>

void filename_split_test()
{
  std::string name = "table_abc_db.json";
  Spliter s(std::string_view{name.c_str(), name.size()});
  s.add_split_ch('_');
  s.add_split_ch('.');

  BOOST_TEST(s.next() == "table");
  BOOST_TEST(s.next() == "abc");
  BOOST_TEST(s.next() == "db");
  BOOST_TEST(s.next() == "json");
  BOOST_TEST(s.eof());
}

void make_filename_test()
{
  std::string_view dir = "hello";
  std::string_view name = "world";
  std::string_view meta = "hello/table_world_meta.json";
  std::string_view data = "hello/table_world_data.db";
  BOOST_TEST(make_data_filename(dir, name) == data);
  BOOST_TEST(make_meta_filename(dir, name) == meta);
}

int main(int, char *[])
{
  filename_split_test();
  make_filename_test();
  return boost::report_errors();
}
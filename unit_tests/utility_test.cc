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

int main(int, char *[])
{
  filename_split_test();
  return boost::report_errors();
}
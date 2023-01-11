#include "common/rc.hpp"
#include <storage/tuple.hpp>
#include <string>
#include <storage/kv/storage.hpp>
#include <storage/tuple.hpp>
#include <boost/core/lightweight_test.hpp>

void basic_test()
{
  long num = 100;
  std::string name = "hello";
  Bytes bytes;
  encode_num(bytes, num);
  encode_varchar(bytes, name);
  TupleMeta meta;

  TupleCellMeta m1;
  TupleCellMeta m2;
  m1.name_ = "num";
  m1.type_ = TupleCellType::INTEGER;
  m2.name_ = "name";
  m2.type_ = TupleCellType::VARCHAR;

  meta.cells_.push_back(m1);
  meta.cells_.push_back(m2);

  Tuple tuple;
  tuple.init(&meta, bytes);

  TupleCell c;
  BOOST_TEST(rc_success(tuple.get_cell(0, c)));
  BOOST_TEST(c.as_integer() == 100);
  BOOST_TEST(rc_success(tuple.get_cell(1, c)));
  BOOST_TEST(c.as_str() == "hello");

  BOOST_TEST(rc_success(tuple.get_cell("num", c)));
  BOOST_TEST(c.as_integer() == 100);
  BOOST_TEST(rc_success(tuple.get_cell("name", c)));
  BOOST_TEST(c.as_str() == "hello");
}

int main(int, char *[])
{
  basic_test();
  return boost::report_errors();
}
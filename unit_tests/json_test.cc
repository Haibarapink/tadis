/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-14 10:43:49
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 04:08:00
 * @FilePath: /tadis/unit_tests/json_test.cc
 * @Description: json 序列化 单元测试
 */
#include "storage/serilze/tuple.ipp"
#include "storage/tuple.hpp"
#include <boost/core/lightweight_test.hpp>
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <cassert>

void tuple_cell_test()
{
  TupleCellMeta cell;
  cell.name_ = "hello world";
  cell.type_ = TupleCellType::CHAR;

  auto v = to_json(cell);

  TupleCellMeta cell2;
  BOOST_TEST(cell2.from_json(v) == RC::SUCCESS);
  BOOST_TEST(cell2.name_ == cell.name_);
  BOOST_TEST(cell2.type_ == cell.type_);
  BOOST_TEST(cell2.len_ == cell.len_);

  TupleMeta meta;
  for (auto i = 0; i < 100; ++i) {
    meta.cells_.push_back(cell);
  }

  v = to_json(meta);
  TupleMeta meta1;
  meta1.from_json(v);
  for (auto &&i : meta1.cells_) {
    BOOST_TEST(i.name_ == cell.name_);
    BOOST_TEST(i.type_ == cell.type_);
    BOOST_TEST(i.len_ == cell.len_);
  }
}

void test()
{
  boost::json::object obj;
  obj.emplace("hello", 1);
  obj.emplace("fuck", 2);
  auto value = boost::json::value{obj};
  auto data = boost::json::serialize(value);

  std::cout << data;

  auto value2 = boost::json::parse(data);

  assert(value2.is_object());
}

int main(int, char *[])
{
  tuple_cell_test();
  test();
  return boost::report_errors();
}
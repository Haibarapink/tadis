/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-09 23:07:40
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-11 20:28:07
 * @FilePath: /tadis/unit_tests/memory_storage_test.cc
 */
#include "storage/kv/storage.hpp"
#include <boost/core/lightweight_test.hpp>
#include <storage/kv/memory.hpp>
#include <memory>
#include <common/utility.hpp>
#include <vector>

void basic_test()
{
  std::unique_ptr<InMemoryStorage> kv{new InMemoryStorage{}};
  std::string k = "hello";
  std::string v = "world";
  kv->set(string2vector(k), string2vector(v));
  Bytes b;
  kv->get(string2vector(k), b);
  BOOST_TEST_EQ(v, vector2string(b));
}

void range_test()
{
  InMemoryStorage kv;
  std::string k = "hello";
  std::string v = "world";
  std::vector<std::pair<std::string, std::string>> vec = {{"hello", "world"}, {"fuck", "world"}};
  for (auto i : vec) {
    kv.set(string2vector(i.first), string2vector(i.second));
  }
  for (auto i : kv) {}
}

int main(int argc, char *argv[])
{
  basic_test();
  range_test();
  return boost::report_errors();
}
/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-09 23:07:40
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-10 19:54:12
 * @FilePath: /tadis/unit_tests/memory_storage_test.cc
 */
#include "storage/storage.hpp"
#include <boost/core/lightweight_test.hpp>
#include <storage/memory.hpp>
#include <memory>
#include <common/utility.hpp>
#include <vector>

void basic_test()
{
  std::unique_ptr<Storage> kv{new InMemoryStorage{}};
  std::string k = "hello";
  std::string v = "world";
  kv->set(string2vector(k), string2vector(v));
  Bytes b;
  kv->get(string2vector(k), b);
  BOOST_TEST_EQ(v, vector2string(b));
}

int main(int argc, char *argv[])
{
  basic_test();
  return boost::report_errors();
}
/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-02 14:05:42
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-02 14:37:34
 * @FilePath: /tadis/unit_tests/lru_test.cc
 * @Description: Test lru replacer
 */
#include "storage/io/replacer.hpp"
#include <cassert>

void test()
{
  LruReplacer<int> replacer{3};
  replacer.put(1);
  replacer.put(2);
  replacer.put(3);
  replacer.put(4);

  assert(!replacer.touch(1));
  replacer.put(1);
  assert(!replacer.touch(2));
  assert(replacer.touch(3));
  replacer.put(2);
  assert(!replacer.touch(4));
}

int main(int, char *[])
{
  test();
}
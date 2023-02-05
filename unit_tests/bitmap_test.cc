/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-03 10:40:55
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-06 00:54:09
 * @FilePath: /tadis/unit_tests/bitmap_test.cc
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 * https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "common/bitmap.hpp"
#include <string>
#include <cassert>
#include <iostream>
#include <vector>

void test()
{
  std::vector<char> view(20, '\0');
  BitMap m{std::string_view{view.data(), view.size()}};
  m.set2one(0);
  m.set2one(1);
  assert(m.get(0) && m.get(1));
  assert(!m.get(8));

  m.set2one(8);
  m.set2zero(0);
  assert(m.get(8) && !m.get(0));
}

void first_after_test()
{
  std::vector<char> view(20, '\0');
  BitMap m{std::string_view{view.data(), view.size()}};
  m.set(10, 1);
  m.set(15, 1);
  m.set(5, 1);
  size_t idx = 0;
  assert(m.first_after(true, 7, idx));
  assert(idx == 10);
  assert(m.first_after(true, 4, idx));
  assert(idx == 5);
  assert(m.first_after(true, 13, idx));
  assert(idx == 15);
  assert(!m.first_after(true, 17, idx));
}

int main(int, char *[])
{
  test();
  first_after_test();
}
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

int main(int, char *[])
{
  test();
}
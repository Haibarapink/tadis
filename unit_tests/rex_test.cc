/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-31 16:58:13
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-31 17:06:03
 * @FilePath: /tadis/unit_tests/rex_test.cc
 */
#include <cassert>
#include <regex>
#include <string_view>

int main(int, char *[])
{
  std::string_view pattern = "table_\\w+_meta.json";
  std::regex rex{pattern.data()};
  std::string_view target = "table_abc_meta.json";
  assert(std::regex_match(target.data(), rex));
}

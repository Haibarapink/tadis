/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-09 23:08:24
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-09 23:12:31
 * @FilePath: /tadis/src/common/unit.hpp
 */
#pragma once

#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <string>
#include <string_view>

template <typename StringType>
inline std::vector<uint8_t> string2vector(const StringType &s)
{
  std::vector<uint8_t> v;
  for (auto ch : s) {
    v.push_back(static_cast<uint8_t>(ch));
  }
  return v;
}

inline std::string vector2string(std::vector<uint8_t> &v)
{
  std::string s;
  for (auto byte : v) {
    s.push_back(static_cast<char>(byte));
  }
  return s;
}
/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-13 23:48:12
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-02 16:34:44
 * @FilePath: /tadis/src/common/bytes.hpp
 */
#pragma once

#include <vector>
#include <span>
#include <cstdint>

using Bytes = std::vector<char>;
using BytesView = std::span<char>;

inline BytesView bytes_view(Bytes &bytes)
{
  return std::span{bytes.data(), bytes.size()};
}
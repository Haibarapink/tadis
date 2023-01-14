/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-13 23:48:12
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-13 23:51:22
 * @FilePath: /tadis/src/common/bytes.hpp
 */
#pragma once

#include <vector>
#include <span>
#include <cstdint>

using Bytes = std::vector<uint8_t>;
using BytesView = std::span<uint8_t>;

inline BytesView bytes_view(Bytes &bytes)
{
  return std::span{bytes.data(), bytes.size()};
}
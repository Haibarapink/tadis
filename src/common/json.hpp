/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-14 00:00:44
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 03:52:18
 * @FilePath: /tadis/src/common/json.hpp
 */
#pragma once

#include "storage/io/fileop.hpp"
#include "common/pson.hpp"

inline auto parse_file2json(std::string_view filename) -> pson::Value
{
  auto bytes = std_readfile(filename);
  pson::Parser p{bytes.data(), bytes.size()};

  pson::Value res;
  p.Parse(res);
  return res;
}
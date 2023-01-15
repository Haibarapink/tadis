/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-14 00:00:44
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-15 12:20:22
 * @FilePath: /tadis/src/common/json.hpp
 */
#pragma once

#include "storage/io/fileop.hpp"
#include <boost/json/src.hpp>

inline auto parse_file2json(std::string_view filename) -> boost::json::value
{
  auto bytes = std_readfile(filename);
  return boost::json::parse(filename);
}
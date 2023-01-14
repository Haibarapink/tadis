/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-02 19:34:49
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-14 10:31:23
 * @FilePath: /tadis/src/common/rc.hpp
 * @Description: rc result code
 */

#pragma once

#include <map>
#include <string_view>

// Result code
enum class RC {
  INTERNAL_ERROR,
  SYNTAX_ERROR,
  KEY_NOT_EXIST,
  TUPLE_CELL_NOT_EXIST,
  JSON_DESERIALIZATION_ERROR,
  SUCCESS,
};

inline bool rc_success(RC rc)
{
  return rc == RC::SUCCESS;
}

// RC to str
static std::string_view rc2str(RC rc)
{
  static std::map<RC, std::string_view> strs = {{RC::INTERNAL_ERROR, "Internal Error"},
      {RC::INTERNAL_ERROR, "Success"},
      {RC::SYNTAX_ERROR, "Syntax error"},
      {RC::KEY_NOT_EXIST, "Key is not existed"},
      {RC::TUPLE_CELL_NOT_EXIST, "Tuple cell is not existed"}};
  return strs[rc];
}
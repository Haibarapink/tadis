/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-02 19:34:49
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-07 16:48:22
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
  OUT_OF_RANGE,
  PAGE_IS_DELETED,
  RECORD_IS_DELETED,
  DB_OPEN_TABLE_ERROR,
  UNINIT,
  TABLE_ALREADY_EXISTED,
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
      {RC::TUPLE_CELL_NOT_EXIST, "Tuple cell is not existed"},
      {RC::JSON_DESERIALIZATION_ERROR, "Json deserialization error"},
      {RC::OUT_OF_RANGE, "Out of range"},
      {RC::PAGE_IS_DELETED, "Page has been deleted"},
      {RC::RECORD_IS_DELETED, "Record has been deleted"},
      {RC::DB_OPEN_TABLE_ERROR, "Db open table error"},
      {RC::UNINIT, "Not init"},
      {RC::TABLE_ALREADY_EXISTED, "Table is already existed"}};
  return strs[rc];
}
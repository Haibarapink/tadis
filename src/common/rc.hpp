#pragma once

#include <map>
#include <string>
#include <string_view>

// Result code
enum class RC : int {
  INTERNAL_ERROR,
  SYNTAX_ERROR,
  KEY_NOT_EXIST,
  COLUMN_NOT_EXIST,
  JSON_DESERIALIZATION_ERROR,
  OUT_OF_RANGE,
  PAGE_IS_DELETED,
  RECORD_IS_DELETED,
  DB_OPEN_TABLE_ERROR,
  UNINIT,
  TABLE_ALREADY_EXISTED,
  TABLE_NOT_EXISTED,
  FETCH_PAGE_ERROR,
  RECORD_TOO_LARGE,
  AMBIGUOUS_COLUMN_NAME,
  SUCCESS,
};

inline bool rc_success(RC rc)
{
  return rc == RC::SUCCESS;
}

// RC to str
static std::string_view rc2str(RC rc)
{
  static std::map<RC, std::string_view> strs;
  if (strs.size() == 0) {
    strs = {{RC::INTERNAL_ERROR, "Internal Error"},
        {RC::INTERNAL_ERROR, "Success"},
        {RC::SYNTAX_ERROR, "Syntax error"},
        {RC::KEY_NOT_EXIST, "Key is not existed"},
        {RC::COLUMN_NOT_EXIST, "Column is not existed"},
        {RC::JSON_DESERIALIZATION_ERROR, "Json deserialization error"},
        {RC::OUT_OF_RANGE, "Out of range"},
        {RC::PAGE_IS_DELETED, "Page has been deleted"},
        {RC::RECORD_IS_DELETED, "Record has been deleted"},
        {RC::DB_OPEN_TABLE_ERROR, "Db open table error"},
        {RC::UNINIT, "Not init"},
        {RC::TABLE_ALREADY_EXISTED, "Table is already existed"},
        {RC::TABLE_NOT_EXISTED, "Table is not existed"},
        {RC::FETCH_PAGE_ERROR, "Fetch page error"},
        {RC::RECORD_TOO_LARGE, "Record is too large"},
        {RC::AMBIGUOUS_COLUMN_NAME, "Ambiguous column name"}};
  }
  return strs[rc];
}
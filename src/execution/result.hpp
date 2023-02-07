/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-07 17:50:29
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-07 18:03:11
 * @FilePath: /tadis/src/execution/result.hpp
 * @Description: Qury result
 */
#pragma once

#include "common/rc.hpp"
#include "storage/tuple.hpp"

#include <vector>

class QueryResult {
public:
  std::vector<Tuple> tuples_;
  RC rc = RC::INTERNAL_ERROR;
};
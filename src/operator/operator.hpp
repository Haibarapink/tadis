/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-03-11 13:28:02
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-03-11 13:31:02
 * @FilePath: /tadis/src/operator/operator.hpp
 * @Description:Operator
 */
#pragma once

#include "storage/tuple.hpp"
#include "common/rc.hpp"
#include <memory>

class Operator {
public:
  Operator() = default;
  ~Operator() = default;

  virtual RC next(std::vector<Tuple> *tuples) = 0;
  virtual bool has_next() = 0;
  virtual RC close() = 0;

protected:
  std::vector<std::unique_ptr<Operator>> children_;
};
/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-09 13:44:17
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-09 23:26:05
 * @FilePath: /tadis/src/sql/operation/operator.hpp
 * @Description: 执行 Operator
 */
#pragma once

#include <common/rc.hpp>
#include <boost/noncopyable.hpp>

class Operator : public boost::noncopyable {
public:
  Operator() = default;

  virtual RC open() = 0;
  virtual RC next() = 0;
  virtual RC close() = 0;
};
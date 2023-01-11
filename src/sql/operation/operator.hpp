/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-09 13:44:17
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-11 13:05:19
 * @FilePath: /tadis/src/sql/operation/operator.hpp
 * @Description: 执行 Operator
 */
#pragma once

#include <common/rc.hpp>
#include <boost/noncopyable.hpp>

template <typename T>
class TOperator : public boost::noncopyable {
public:
  RC open()
  {
    return ((T *)this)->open();
  }

  RC next()
  {
    return ((T *)this)->next();
  }

  RC close()
  {
    return ((T *)this)->close();
  }
};

class Operator : public boost::noncopyable {
public:
  virtual RC open() = 0;
  virtual RC next() = 0;
  virtual RC close() = 0;
};
/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 12:54:19
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-11 13:59:32
 * @FilePath: /tadis/src/sql/execution/execution.hpp
 * @Description: 执行层, 解析完毕后传入Operator，在此执行
 */
#pragma once

#include <algorithm>
#include <sql/operation/operator.hpp>

template <typename OperatorType>
class Executer {
public:
  Executer(OperatorType op) : op_(std::move(op))
  {}

  void exec();

private:
  OperatorType op_;
};
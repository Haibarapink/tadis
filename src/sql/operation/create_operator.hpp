/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:00:35
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-15 11:28:39
 * @FilePath: /tadis/src/sql/operation/create_operator.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 */
#pragma once

#include "sql/context/create_table_ctx.hpp"

class CreateTableOp {
public:
  CreateTableOp(CreateTableCtx ctx) : ctx_(std::move(ctx))
  {}

  RC open();
  RC next();
  RC close();

private:
  CreateTableCtx ctx_;
};
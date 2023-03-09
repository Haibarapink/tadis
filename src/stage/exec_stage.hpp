/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-03-09 15:09:52
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-03-09 16:09:13
 * @FilePath: /tadis/src/stage/exec_stage.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 * https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "stage/stage.hpp"
#include "sql/parser/ast.hpp"

class ExecStage : public Stage {
public:
  ExecStage()
  {
    name_ = "ExecStage";
  }

  void init(QueryAst *query)
  {
    query_ = query;
  }

private:
  QueryAst *query_;
};
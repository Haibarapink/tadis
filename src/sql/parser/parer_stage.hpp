/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-03-09 14:59:49
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-03-11 09:32:17
 * @FilePath: /tadis/src/stage/parsing_stage.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 */
#pragma once

#include "catalog/catalog.hpp"
#include "operator/create_table_op.hpp"
#include "operator/operator.hpp"
#include "sql/parser/ast.hpp"
#include "sql/parser/parser.hpp"
#include "execution/exec_stage.hpp"
#include "stage/stage.hpp"
#include "statement/create_table_stmt.hpp"
#include "operator/insert_op.hpp"
#include <variant>

class ParsingStage : public Stage {
public:
  ParsingStage()
  {
    name_ = "ParsingStage";
  }

  void init(std::string query_str)
  {
    query_ = std::move(query_str);
  }

  RC handle_event()
  {
    Parser<std::string_view> parser{query_};

    RC rc = parser.parse();
    if (rc != RC::SUCCESS) {
      return rc;
    }

    Stage *exec = new ExecStage{};
    auto &ast = parser.query();
    if (std::holds_alternative<SelectAst>(ast)) {

    } else if (std::holds_alternative<CreateTableAst>(ast)) {
      auto &cs = const_cast<CreateTableAst &>(std::get<CreateTableAst>(ast));
      CreateTbaleStmt stmt;
      stmt.table_name = std::move(cs.table_name_);
      stmt.cols_ = std::move(cs.col_attrs_);
      Operator *op = new CreateTableOp{std::move(stmt)};
      ((ExecStage *)(exec))->set_op(op);
    } else if (std::holds_alternative<DeleteAst>(ast)) {

    } else if (std::holds_alternative<InsertAst>(ast)) {

      auto &is = const_cast<InsertAst &>(std::get<InsertAst>(ast));
      InsertStmt stmt;
      auto tm = Catalog::catalog().table_manager();
      if (!tm->contain(is.table_name_)) {
        return RC::TABLE_NOT_EXISTED;
      }
      stmt.table_ = tm->table(is.table_name_);
      stmt.values_ = std::move(is.values_);
      Operator *op = new InsertOp{std::move(stmt)};
      ((ExecStage *)(exec))->set_op(op);
    }

    this->set_next(exec);

    return rc;
  }

private:
  std::string query_;
};
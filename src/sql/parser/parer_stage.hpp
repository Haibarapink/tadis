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
#include "common/logger.hpp"
#include "operator/create_table_op.hpp"
#include "operator/operator.hpp"
#include "operator/table_scan_op.hpp"
#include "sql/parser/ast.hpp"
#include "sql/parser/parser.hpp"
#include "execution/exec_stage.hpp"
#include "stage/stage.hpp"
#include "statement/create_table_stmt.hpp"
#include "operator/insert_op.hpp"
#include "operator/select_op.hpp"
#include "statement/filter_stmt.hpp"

#include <bits/types/FILE.h>
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
    auto table_manager = Catalog::catalog().table_manager();
    auto &ast = parser.query();
    if (std::holds_alternative<SelectAst>(ast)) {
      //
      // Select Op
      //
      auto &ss = const_cast<SelectAst &>(std::get<SelectAst>(ast));
      Operator *op = new SelectOp{};

      if (!table_manager->contain(ss.from_list_[0])) {
        return RC::TABLE_NOT_EXISTED;
      }

      Operator *table_scan_op = new TableScanOp{table_manager->table(ss.from_list_[0])};

      // check conditions' attributes
      for (auto &cnd : ss.cond_list_) {
        if (cnd.left_.type_ != AttrType::REL_ATTR && cnd.right_.type_ != AttrType::REL_ATTR) {
          continue;
        }
        auto &&c = cnd.left_.type_ == AttrType::REL_ATTR ? cnd.left_ : cnd.right_;
        auto &&attr = std::any_cast<RelAttr &>(c.value_);

        if (attr.table_ != ss.from_list_[0]) {
          LOG_WARN << "attr.table_ != ss.from_list_[0]";
          return RC::TABLE_NOT_EXISTED;
        }
      }

      if (!ss.cond_list_.empty()) {
        FilterStmt filter_stmt{std::move(ss.cond_list_)};
        Operator *filter_op = new FilterOp{std::move(filter_stmt)};

        filter_op->add_child(table_scan_op);
        op->add_child(filter_op);
      } else {
        op->add_child(table_scan_op);
      }

      ((ExecStage *)(exec))->set_op(op);
      LOG_DEBUG << "Select Op";
    } else if (std::holds_alternative<CreateTableAst>(ast)) {
      //
      // Create Table Op
      //
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
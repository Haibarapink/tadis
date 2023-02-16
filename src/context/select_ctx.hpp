/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-11 14:01:41
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 03:22:51
 * @FilePath: /tadis/src/sql/context/select_ctx.hpp
 * @Description:
 */
#pragma once

#include "sql/parser/ast.hpp"
#include "session/session.hpp"
#include "execution/result.hpp"
#include "storage/table.hpp"
#include "storage/tuple.hpp"

#include <map>

class SelectContext {
public:
  RC init(SelectAst &ast);
  RC exec(QueryResult &res);

private:
  SelectAst *query_ = nullptr;
  Table *table_ = nullptr;

  TupleFilter filter_;

};

inline RC SelectContext::init(SelectAst &ast)
{
  auto tm = GlobalSession::global_session().table_manager();

  if (ast.from_list_.empty() || ast.selist_.empty()) {
    return RC::INTERNAL_ERROR;
  }

  if (!tm->contain(ast.from_list_[0])) {
    return RC::TABLE_NOT_EXISTED;
  }

  table_ = tm->table(ast.from_list_[0]);
  query_ = &ast;

  auto&& tuple_meta = table_->table_meta().tuple_meta();
  for (auto &&cnd : ast.cond_list_) {
    if ((cnd.left_.type_ == AttrType::REL_ATTR && cnd.right_.type_ == AttrType::REL_ATTR)
        || (cnd.left_.type_ != AttrType::REL_ATTR && cnd.right_.type_ != AttrType::REL_ATTR)) {
      LOG_WARN << "left is REL_ATTR and right is REL_ATTR";
      return RC::SYNTAX_ERROR;
    }

    auto&& table_name = cnd.left_.type_ == AttrType::REL_ATTR ? std::any_cast<RelAttr&>(cnd.left_.value_).table_ :
                                                              std::any_cast<RelAttr&>(cnd.right_.value_).table_;
    auto&& attr_name = cnd.left_.type_ == AttrType::REL_ATTR ? std::any_cast<RelAttr&>(cnd.left_.value_).attribute_ :
                                                        std::any_cast<RelAttr&>(cnd.right_.value_).attribute_;

    if (table_name != table_->table_meta().name()) {
      return RC::TABLE_NOT_EXISTED;
    }

    long idx = -1;
    for (auto i = 0; i < tuple_meta.cells_.size() ; ++i)
    {
      if (tuple_meta.cells_[i].name_ == attr_name) {
        idx = i;
        break;
      }
    }

    if (idx == -1) {
      return RC::TUPLE_CELL_NOT_EXIST;
    }

    // 比较的时候是反着比的
    if (cnd.left_.type_ != AttrType::REL_ATTR) {
      CondOp op = cnd.op_;
      if (op == CondOp::GREATER ) {
        op = CondOp::SMALLER;
      } else if (op == CondOp::SMALLER) {
        op = CondOp::GREATER;
      }
      TupleCellFilter cell_filter{std::move(cnd.left_), op};
      filter_.add_cell_filter(idx, std::move(cell_filter));
    } else {
      TupleCellFilter cell_filter { std::move(cnd.right_), cnd.op_};
      filter_.add_cell_filter(idx, std::move(cell_filter));
    }
  }


  return RC::SUCCESS;
}

// 暂时全部输出
inline RC SelectContext::exec(QueryResult &res)
{
  res.tuples_ = table_->tuples();
  res.rc = RC::SUCCESS;
  return RC::SUCCESS;
}
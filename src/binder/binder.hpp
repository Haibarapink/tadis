#pragma once

#include "sql/stmt.hpp"
#include "execution/execution.hpp"
#include "execution/executer_context.hpp"
#include "execution/projection_executer.hpp"
#include "execution/filter_executer.hpp"
#include "execution/insert_executer.hpp"
#include "expression/column_value_expression.hpp"
#include "expression/constant_expression.hpp"
#include "expression/projection_expression.hpp"
#include "expression/comparsion_expression.hpp"
#include "expression/filter_expression.hpp"
#include "execution/seqscan_executer.hpp"
#include "execution/create_table_execution.hpp"
#include "execution/error_executer.hpp"

// bind a statement to an operator
class Binder {
  friend class BinderTest;
public:
    Binder(ExecuterContext *ctx): ctx_(ctx) {}
    ~Binder() = default;

    // bind a statement to an executer
    Executer *bind(SqlStmt *stmt);
private:
    Executer* error_executer(RC rc,  std::string msg = "", QueryType type = QueryType::SELECT) {
      Executer* error_exec = new ErrorExecuter{rc, &ctx_->result_};
      ctx_->result_.msg_ = std::move(msg);
      ctx_->result_.rc_ = rc;
      ctx_->result_.type_ = type;
      return error_exec;
    }

    Executer* bind_select(SqlStmt* select_stmt) {
      assert(ctx_);
      ctx_->result_.type_ = QueryType::SELECT;
      auto& stmt = select_stmt->stmt<SelectStmt>();
      auto& select_list = stmt.selist_;
      auto& where_list = stmt.cond_list_;
      auto& table_list = stmt.from_list_;
      auto catalog = ctx_->catalog();
      assert(catalog);
      if (select_list.empty() || table_list.empty()) {
        return error_executer(RC::SYNTAX_ERROR);
      }

      if (table_list.size() > 1) {
        return error_executer(RC::SYNTAX_ERROR , "Not support syntax");
      }

      Executer* executer = nullptr;
      Executer* filter_exec = nullptr;
      Executer* scan_exec = nullptr;

      auto &table = table_list[0];
      auto t = catalog->get_table(table);

      for (auto &attr : select_list) {
        if (!attr.table_.empty() && table != attr.table_) {
          std::string msg = attr.table_ + std::string(" can't bind to ") + table;
          return error_executer(RC::SYNTAX_ERROR, std::move(msg));
        }
      }

      if (!catalog->has_table(table)) {
        return error_executer(RC::TABLE_NOT_EXISTED);
      }

      Schema *table_schema = new Schema{t->schema()};
      scan_exec = new SeqScanExectuer{catalog->scanner(table), table_schema};

      if (!where_list.empty()) {
        filter_exec = filter_executer(table, where_list);
        filter_exec->children_[0].reset(scan_exec);
        filter_exec->schema_.reset(new Schema{t->schema()});
      }

      // out put
      std::vector<std::unique_ptr<Expression>> cvs;
      std::vector<size_t> output_schema_idx;
      for (auto & attr : select_list) {
        if (attr.attribute_ == "*") {
          for (size_t i = 0; i < t->schema().columns_.size(); ++i) {
            cvs.emplace_back(new ColumnValueExpression{i});
            output_schema_idx.push_back(i);
          }
        } else {
          int idx = t->schema().column_idx(attr.attribute_);
          if (idx==-1) {
            return error_executer(RC::COLUMN_NOT_EXIST);
          }
          cvs.emplace_back(new ColumnValueExpression{static_cast<size_t>(idx)});
          output_schema_idx.push_back(idx);
        }
      }

      Schema* output_schema = new Schema{Schema::copy_schema(&t->schema(), output_schema_idx)};
      ProjectionExpression* pj_expr = new ProjectionExpression{std::move(cvs)};
      executer = new ProjecetionExecuter{};
      executer->schema_.reset(new Schema{*output_schema});
      executer->expression_ = std::unique_ptr<Expression>{pj_expr};
      ctx_->result_.output_schema_.reset(output_schema);

      if (filter_exec) {
        executer->children_.emplace_back(filter_exec);
      } else {
        executer->children_.emplace_back(scan_exec);
      }

      return executer;
    }

    Executer *filter_executer(const std::string& table, const std::vector<Condition>& cond_list) {
      auto catalog = ctx_->catalog();
      assert(catalog->has_table(table));

      Expression* filter_expr = nullptr;
      std::vector<std::unique_ptr<Expression>> children;

      Expression* left = nullptr, *right = nullptr;
      auto t = catalog->get_table(table);
      for (auto &cnd : cond_list) {
        if (cnd.left_.type_ == AttrType::REL_ATTR) {
          RelAttr a = std::any_cast<RelAttr>(cnd.left_.value_);
          left  = col_value_express(t, a.attribute_);
        } else {
          left = constant_express(cnd.left_);
        }

        if (cnd.right_.type_ == AttrType::REL_ATTR) {
          RelAttr a = std::any_cast<RelAttr>(cnd.right_.value_);
          right  = col_value_express(t, a.attribute_);
        } else {
          right = constant_express(cnd.right_);
        }
        ComparsionType type;
        assert(convert(cnd.op_, type));
        ComparsionExpression* cmp = new ComparsionExpression{left, right, type};
        children.emplace_back(cmp);
      }

      filter_expr = new FilterExpression{std::move(children)};
      FilterExecuter* executer = new FilterExecuter{filter_expr, nullptr};
      return executer;
    }

    Expression* col_value_express(Table* table, const std::string& attr) {
      auto idx = table->schema().column_idx(attr);
      if (idx < 0) {
        assert(false && "attribute don't contain");
        return nullptr;
      }
      ColumnValueExpression* expr = new ColumnValueExpression{static_cast<size_t>(idx)};
      return expr;
    }

    Expression* constant_express(const Value& v) {
      assert(v.type_ != AttrType::REL_ATTR);
      ConstantExpression* expr = new ConstantExpression{v};
      return expr;
    }

    Executer* bind_insert(SqlStmt* insert_stmt) {
      ctx_->result_.type_ = QueryType::INSERT;
      auto& insert = insert_stmt->stmt<InsertStmt>();
      auto catalog = ctx_->catalog();
      auto table = catalog->get_table(insert.table_name_);

      if (table == nullptr) {
        return error_executer(RC::TABLE_NOT_EXISTED, "", QueryType::INSERT);
      }

      auto stm = catalog->store_manager();

      // check columns' type
      auto& schema = table->schema();

      if (insert.values_.size() != schema.columns_.size()) {
        return error_executer(RC::COLUMN_TYPE_MISMATCH, "", QueryType::INSERT);
      }

      for (auto i = 0 ; i < schema.columns_.size(); ++i) {
        // check type
        auto& col = schema.columns_.at(i);
        auto& val = insert.values_.at(i);
        if (!check_attr_type(val.type_, col.type())) {
          return error_executer(RC::COLUMN_TYPE_MISMATCH, "", QueryType::INSERT);
        }
      }

      Executer* executer = new InsertExecuter{stm, table, &table->schema(), std::move(insert.values_)};
 
      return executer;
    }

    Executer* bind_delete(SqlStmt* delete_stmt) {
      ctx_->result_.type_ = QueryType::DELETE;
      return error_executer(RC::SYNTAX_ERROR, "", QueryType::DELETE);
    }

    Executer* bind_drop(SqlStmt* drop_stmt) {
      ctx_->result_.type_ = QueryType::DROP;
      return error_executer(RC::SYNTAX_ERROR, "", QueryType::DROP);
    }

    Executer* bind_create(SqlStmt* create_stmt) {
      ctx_->result_.type_ = QueryType::CREATE;
      auto catalog = ctx_->catalog();
      auto& create_table = create_stmt->stmt<CreateTableStmt>();
      if (catalog->has_table(create_table.table_name_)) {
        return error_executer(RC::TABLE_ALREADY_EXISTED, "", QueryType::CREATE);
      }

      if (create_table.col_attrs_.empty()) {
        return error_executer(RC::SYNTAX_ERROR, "empty attributes while create table", QueryType::CREATE);
      }

      std::vector<Column> cols;
      for (auto i = 0; i < create_table.col_attrs_.size() ; ++i )
      {
        ColumnType col_type;
        auto &col = create_table.col_attrs_[i];
        if (col.type_ == "VARCHAR") {
          col_type = ColumnType::VARCHAR;
        } else if (col.type_ == "CHAR") {
          col_type = ColumnType::CHAR;
        } else if (col.type_ == "INT") {
          col_type = ColumnType::INT;
        } else if (col.type_ == "FLOAT") {
          col_type = ColumnType::FLOAT;
        } else {
          assert(false);
        }

        cols.emplace_back(col_type, col.name_, col_type == ColumnType::CHAR ? col.size_ : 0);
      }

      Executer * executer = new CreateTableExecuter{create_table.table_name_, std::move(cols), catalog};
      return executer;
    }


    ExecuterContext* ctx_;
};

//@brief user should control the lifetime of Exectuer
inline Executer *Binder::bind(SqlStmt *stmt)
{
  Executer *res = nullptr;
  switch (stmt->type()) {
    case QueryType::SELECT:
      return bind_select(stmt);
    case QueryType::INSERT:
      return bind_insert(stmt);
    case QueryType::DELETE:
      return bind_delete(stmt);
    case QueryType::DROP:
      return bind_drop(stmt);
    case QueryType::CREATE:
      return bind_create(stmt);
    default:
      assert(false && "not implement");
  }
  return res;
}
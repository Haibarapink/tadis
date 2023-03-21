#pragma once

#include "common/logger.hpp"
#include "common/rc.hpp"
#include "operator/operator.hpp"
#include "sql/parser/ast.hpp"
#include "statement/select_stmt.hpp"
#include "statement/filter_stmt.hpp"
#include "storage/tuple.hpp"
#include <memory>

class FilterOp : public Operator {
public:
  FilterOp(FilterStmt stmt) : stmt_(stmt)
  {}

  void add_children(Operator *op)
  {
    this->children_.push_back(std::unique_ptr<Operator>{op});
  }

  RC next(std::vector<Tuple> *tuple)
  {
    done_ = true;
    std::vector<Tuple> sub_tuples;

    if (!this->children_.empty() && this->children_[0]->has_next()) {
      this->children_[0]->next(&sub_tuples);
    }

    for (auto &t : sub_tuples) {
      size_t p = 0;
      for (auto &cnd : stmt_.cnds_) {
        if (cnd.right_.type_ == AttrType::REL_ATTR && cnd.left_.type_ != AttrType::REL_ATTR) {
          // value : attr
          TupleCell cell;
          auto ok = t.get_cell(p, cell);

          if (!rc_success(ok))
            return ok;

          int cmp_res = Comparator::cmp(cnd.left_, cell);
          switch (cnd.op_) {
            case CondOp::EQ:
              if (cmp_res == 0)
                tuple->push_back(t);
              break;
            case CondOp::GREATER:
              if (cmp_res > 0)
                tuple->push_back(t);
              break;
            case CondOp::SMALLER:
              if (cmp_res < 0)
                tuple->push_back(t);
              break;
            case CondOp::NOT_EQ:
              if (cmp_res != 0)
                tuple->push_back(t);
              break;
            default:
              LOG_WARN << "not support other CondOp...";
              // do nothing
          }

        } else {
          // ?
          // value : value
        }
        p++;
      }
    }

    return RC::SUCCESS;
  }

  bool has_next()
  {
    return done_ == false;
  }

  RC close()
  {
    return RC::SUCCESS;
  }

private:
  bool done_ = false;

  FilterStmt stmt_;
};

class SelectOp : public Operator {
public:
  RC next(std::vector<Tuple> *tuples)
  {
    done_ = true;
    return children_[0]->next(tuples);
  }

  bool has_next()
  {
    return done_ == false;
  }

  RC close()
  {
    return RC::SUCCESS;
  }

private:
  bool done_ = false;
};
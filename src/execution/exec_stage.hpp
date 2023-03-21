/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-03-09 15:09:52
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-03-11 13:27:32
 * @FilePath: /tadis/src/stage/exec_stage.hpp
 * @Description:
 */
#pragma once

#include "common/logger.hpp"
#include "common/rc.hpp"
#include "operator/operator.hpp"
#include "stage/stage.hpp"
#include "statement/insert_stmt.hpp"
#include <vector>

class ExecStage : public Stage {
public:
  ExecStage()
  {
    name_ = "ExecStage";
  }

  RC handle_event()
  {
    assert(op_);
    std::vector<Tuple> res;
    if (op_->has_next()) {
      RC rc = op_->next(&res);
      if (!rc_success(rc)) {
        LOG_WARN << rc2str(rc);
      }
    }
    return RC::SUCCESS;
  }

  void set_op(Operator *op)
  {
    op_.reset(op);
  }

private:
  std::unique_ptr<Operator> op_;
};
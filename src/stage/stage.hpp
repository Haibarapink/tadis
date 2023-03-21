/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-03-09 14:48:38
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-03-09 15:23:10
 * @FilePath: /tadis/src/stage/stage.hpp
 * @Description: Stage
 */
#pragma once

#include "common/rc.hpp"
#include "common/logger.hpp"
#include "common/noncopyable.hpp"

#include <string>

class Stage : public noncopyable {
public:
  Stage() = default;
  ~Stage() = default;

  RC do_request()
  {
    auto rc = handle_event();

    if (rc != RC::SUCCESS) {
      return rc;
    }

    if (next_) {
      return next_->do_request();
    }
    return RC::SUCCESS;
  }

  // 管理下一个Stage的生命周期
  void set_next(Stage *next)
  {
    next_.reset(next);
  }

  virtual RC handle_event() = 0;

protected:
  std::string name_ = "Unknow";
  std::unique_ptr<Stage> next_ = nullptr;
};
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
  RC do_request()
  {
    auto rc = handle_event();

    if (rc != RC::SUCCESS) {
      return rc;
    }

    if (next_) {
      return next_->handle_event();
    }
    return RC::SUCCESS;
  }

  void set_next(Stage *next)
  {
    next_ = next;
  }

  virtual RC handle_event();

protected:
  std::string name_ = "Unknow";
  Stage *next_ = nullptr;
};
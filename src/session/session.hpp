/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-31 19:58:05
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-03-09 14:54:50
 * @FilePath: /tadis/src/session/session.hpp
 * @Description: ...
 */
#pragma once

#include "storage/table_manager.hpp"

#include <memory>

class GlobalSession {
public:
  RC init(std::string_view base_dir)
  {
    tm_ = std::unique_ptr<TableManager>{new TableManager{}};
    return tm_->init(base_dir);
  }

  TableManager *table_manager()
  {
    return tm_.get();
  }

  // Get gloabl session
  static GlobalSession &global_session()
  {
    // single in cpp , hhhh
    static GlobalSession s;
    return s;
  }

private:
  std::unique_ptr<TableManager> tm_ = nullptr;
};

// Helper function
inline RC init_gloabl_session(std::string_view path)
{
  auto &&session = GlobalSession::global_session();
  return session.init(path);
}

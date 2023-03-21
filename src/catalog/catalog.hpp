/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-31 19:58:05
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-03-09 14:54:50
 * @Description: ...
 */
#pragma once

#include "storage/table_manager.hpp"

#include <memory>

class Catalog {
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

  static Catalog &catalog()
  {
    static Catalog s;
    return s;
  }

private:
  std::unique_ptr<TableManager> tm_ = nullptr;
};

// Helper function
inline RC init(std::string_view path)
{
  auto &&catalog = Catalog::catalog();
  return catalog.init(path);
}

/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-02 16:08:55
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-05 00:40:20
 * @FilePath: /tadis/src/storage/io/iodef.hpp
 * @Description Defines
 */
#pragma once

#include "common/bytes.hpp"
#include <cstddef>
#include <sstream>
#include <string>

constexpr size_t PAGESIZE = 4096;

using PageId = std::size_t;

class RecordId {
public:
  PageId page_id_;
  size_t slot_id_;

  std::string to_string()
  {
    std::string res;
    std::stringstream ss{res};
    ss << "PageId : " << page_id_ << " SlotId: " << slot_id_;
    return res;
  }
};

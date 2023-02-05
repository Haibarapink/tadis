/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-02 16:08:55
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-05 15:10:43
 * @FilePath: /tadis/src/storage/io/iodef.hpp
 * @Description Defines
 */
#pragma once

#include "common/bytes.hpp"
#include <cstddef>
#include <sstream>
#include <string>

constexpr size_t PAGESIZE = 4096;
constexpr size_t BFP_MAX_PAGE_NUM = (4096 - sizeof(size_t) * 2) * 8;
constexpr size_t BFP_BITMAP_MAX_SIZE = (4096 - sizeof(size_t) * 2);

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

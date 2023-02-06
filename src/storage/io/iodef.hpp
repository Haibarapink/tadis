/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-02 16:08:55
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-06 13:57:12
 * @FilePath: /tadis/src/storage/io/iodef.hpp
 * @Description Defines
 */
#pragma once

#include "common/bytes.hpp"
#include <cstddef>
#include <sstream>
#include <string>

constexpr size_t INVALID_ID = SIZE_MAX;
constexpr size_t PAGESIZE = 4096;
constexpr size_t RECORD_MAX_SIZE = PAGESIZE - sizeof(size_t) * 4;
constexpr size_t BFP_MAX_PAGE_NUM = (4096 - sizeof(size_t) * 2) * 8;
constexpr size_t BFP_BITMAP_MAX_SIZE = (4096 - sizeof(size_t) * 2);

using PageId = std::size_t;

class RecordId {
public:
  PageId page_id_ = INVALID_ID;
  size_t slot_id_ = INVALID_ID;

  RecordId() = default;
  ~RecordId() = default;

  RecordId(const RecordId &rid)
  {
    page_id_ = rid.page_id_;
    slot_id_ = rid.slot_id_;
  }

  RecordId &operator=(const RecordId &rid)
  {
    page_id_ = rid.page_id_;
    slot_id_ = rid.slot_id_;
    return *this;
  }

  RecordId(RecordId &&rid)
  {
    page_id_ = rid.page_id_;
    slot_id_ = rid.slot_id_;
  }

  RecordId &operator=(RecordId &&rid)
  {
    page_id_ = rid.page_id_;
    slot_id_ = rid.slot_id_;
    return *this;
  }

  bool operator==(const RecordId &r)
  {
    return (page_id_ == r.page_id_ && slot_id_ == r.slot_id_);
  }

  bool operator!=(const RecordId &r)
  {
    return (page_id_ != r.page_id_ || slot_id_ != r.slot_id_);
  }

  std::string to_string()
  {
    std::string res;
    std::stringstream ss{res};
    ss << "PageId : " << page_id_ << " SlotId: " << slot_id_;
    return res;
  }
};

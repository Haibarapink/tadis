#pragma once

#include <cstddef>
#include <sstream>
#include <string>

constexpr size_t INVALID_ID = SIZE_MAX;
constexpr size_t PAGE_SIZE = 1024;
constexpr size_t RECORD_MAX_SIZE = PAGE_SIZE - sizeof(size_t) * 4;
constexpr size_t BFP_MAX_PAGE_NUM = (PAGE_SIZE - sizeof(size_t) * 2) * 8;
constexpr size_t BFP_BITMAP_MAX_SIZE = (PAGE_SIZE - sizeof(size_t) * 2);

using PageId = std::size_t;

class RID {
public:
 PageId page_id_ = INVALID_ID;
 size_t slot_id_ = INVALID_ID;

 RID() = default;
 ~RID() = default;

 RID(const RID &rid)
 {
   page_id_ = rid.page_id_;
   slot_id_ = rid.slot_id_;
 }

 RID &operator=(const RID &rid)
 {
   page_id_ = rid.page_id_;
   slot_id_ = rid.slot_id_;
   return *this;
 }

 RID(RID &&rid)
 {
   page_id_ = rid.page_id_;
   slot_id_ = rid.slot_id_;
 }

 RID &operator=(RID &&rid)
 {
   page_id_ = rid.page_id_;
   slot_id_ = rid.slot_id_;
   return *this;
 }

 bool operator==(const RID &r)
 {
   return (page_id_ == r.page_id_ && slot_id_ == r.slot_id_);
 }

 bool operator!=(const RID &r)
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
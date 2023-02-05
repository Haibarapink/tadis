/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-04 15:55:55
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-05 20:23:06
 * @FilePath: /tadis/src/storage/io/record.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 * https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "common/bytes.hpp"
#include "common/logger.hpp"
#include "storage/io/buffer_pool.hpp"
#include "storage/io/iodef.hpp"

#include <string>
#include <sstream>
#include <cassert>
#include <cstring>
#include <math.h>

class Record;
class RecordView;
//  On disk :
//  |free_addr_start_|free_addr_end_|rec_idx_count_|...rec_idx...|...free...|rec3|  rec2  | rec1 |
//                               idx of record start
//
class RecordPage {
public:
  friend class PageRecordScanner;

  std::string to_string()
  {
    std::string res;
    std::stringstream ss{res};
    ss << "free_addr_start_ : " << free_addr_start_ << " free_addr_end_ : " << free_addr_end_ << " rec_idx_count_ "
       << rec_idx_count_;

    return res;
  }

  void init(Page *p)
  {
    assert(p);
    auto data = p->data();
    memcpy(reinterpret_cast<char *>(&free_addr_start_), data, sizeof(size_t));
    memcpy(reinterpret_cast<char *>(&free_addr_end_), data + sizeof(size_t), sizeof(size_t));
    memcpy(reinterpret_cast<char *>(&rec_idx_count_), data + 2 * sizeof(size_t), sizeof(size_t));
    if (free_addr_end_ == 0) {
      // first time ==
      free_addr_end_ = PAGESIZE;
      memcpy(data + sizeof(size_t), reinterpret_cast<char *>(&free_addr_end_), sizeof(size_t));
    }

    for (size_t i = 0; i < rec_idx_count_; ++i) {
      size_t idx{0};
      memcpy(reinterpret_cast<char *>(&idx), data + (3 + i) * sizeof(size_t), sizeof(size_t));
      rec_idx_.push_back(idx);
    }
    page_ = p;

    LOG_DEBUG << "init " << to_string();
  }

  bool insert(const Record &rec, RecordId &rid);
  bool remove(const RecordId &rid);
  bool get(Record &rec, const RecordId &rid);
  bool get(RecordView &rec, const RecordId &rid);

  /**
   * @brief
   *  判断该RID是否存在 record
   * @return
   *   返回 OUT_OF_RANGE 表示扫描到 Page的结尾，不存在。
   *   返回 RECORD_IS_DELETED 表示扫描到一个被标记delete的record，也是不存在
   *  否则返回 RC::SUCCESS
   */
  RC contain(const RecordId &rid);

  void write_head()
  {
    auto data = page_->data();
    memcpy(data, reinterpret_cast<char *>(&free_addr_start_), sizeof(size_t));
    memcpy(data + sizeof(size_t), reinterpret_cast<char *>(&free_addr_end_), sizeof(size_t));
    memcpy(data + sizeof(size_t) * 2, reinterpret_cast<char *>(&rec_idx_count_), sizeof(size_t));
    for (size_t i = 0; i < rec_idx_.size(); ++i) {
      memcpy(data + sizeof(size_t) * (3 + i), reinterpret_cast<char *>(&rec_idx_[i]), sizeof(size_t));
      if (sizeof(size_t) * (3 + i) >= rec_idx_[i]) {
        assert(false);
      }
    }
  }

  // 插入后使用
  void write_head(const RecordId &rid, size_t idx)
  {
    auto data = page_->data();
    memcpy(data, reinterpret_cast<char *>(&free_addr_start_), sizeof(size_t));
    memcpy(data + sizeof(size_t), reinterpret_cast<char *>(&free_addr_end_), sizeof(size_t));
    memcpy(data + sizeof(size_t) * 2, reinterpret_cast<char *>(&rec_idx_count_), sizeof(size_t));

    LOG_DEBUG << "recordIdx offset : " << sizeof(size_t) * (3 + rid.slot_id_) << " idx : " << idx;

    memcpy(data + sizeof(size_t) * (3 + rid.slot_id_), reinterpret_cast<char *>(&idx), sizeof(size_t));
  }

private:
  bool enough(size_t record_size)
  {
    size_t start_idx = free_addr_end_ - record_size - 1;
    size_t offset = sizeof(size_t) * (3 + rec_idx_.size());
    return offset + sizeof(size_t) < start_idx;
  }

  size_t remain_free_size() const
  {
    return free_addr_end_ - free_addr_start_;
  }

  size_t free_addr_start_ = 0;
  size_t free_addr_end_ = PAGESIZE;
  size_t rec_idx_count_ = 0;
  std::vector<size_t> rec_idx_;

  Page *page_ = nullptr;
};

// On disk : |is_delete_|....|
class Record {
public:
  friend class RecordPage;

  // a record
  bool is_deleted() const
  {
    return is_deleted_;
  }

  auto rid()
  {
    return rid_;
  }

  Bytes &data()
  {
    return data_;
  }

private:
  bool is_deleted_ = false;
  RecordId rid_;
  Bytes data_;
};

// On disk : |is_delete_|....|
class RecordView {
public:
  friend class RecordPage;

  // a record
  bool is_deleted() const
  {
    return is_deleted_;
  }

  auto rid()
  {
    return rid_;
  }

  BytesView &data()
  {
    return data_;
  }

private:
  bool is_deleted_ = false;
  RecordId rid_;
  BytesView data_;
};

/** @brief 负责扫描一个Page的record **/
class PageRecordScanner {
public:
  void init(RecordPage *rp)
  {
    rp_ = rp;
    next_rid_.page_id_ = rp->page_->pid();
    next_rid_.slot_id_ = 0;
  }

  bool has_next() const
  {
    return rp_->contain(next_rid_) != RC::OUT_OF_RANGE;
  }

  template <typename RecordType>
  RC next(RecordType &rec, RecordId &rid)
  {
    while (true) {
      auto rc = rp_->get(rec, next_rid_);
      if (rc == RC::OUT_OF_RANGE) {
        return rc;
      } else if (rc == RC::SUCCESS) {
        next_rid_.slot_id_++;
        return rc;
      } else {
        // current record has been deleted
        next_rid_.slot_id_++;
      }
    }
    return RC::SUCCESS;
  }

private:
  RecordPage *rp_;
  RecordId next_rid_;
};

/** @brief 负责扫描整个table的record **/
class RecordScanner {
public:
private:
};

inline bool RecordPage::insert(const Record &rec, RecordId &rid)
{
  if (!enough(rec.data_.size())) {
    return false;
  }

  size_t start_idx = free_addr_end_ - rec.data_.size() - 1;
  char *start = page_->data() + start_idx;

  // set delete mark
  start[0] = 0;

  // copy data
  memcpy(start + 1, rec.data_.data(), rec.data_.size());

  free_addr_start_++;
  free_addr_end_ = start_idx;

  rid.page_id_ = page_->pid();
  rid.slot_id_ = rec_idx_.size();

  rec_idx_count_++;
  rec_idx_.push_back(start_idx);

  write_head(rid, start_idx);
  return true;
}

inline bool RecordPage::remove(const RecordId &rid)
{
  // only mark delete ?
  RecordView rec;
  if (!get(rec, rid)) {
    return false;
  }
  rec.data_.data()[0] = 1;
  write_head();

  // TODO mark the deleted record in a map

  return true;
}

inline RC RecordPage::contain(const RecordId &rid)
{
  if (rec_idx_.empty() || rec_idx_.size() <= rid.slot_id_) {
    return RC::OUT_OF_RANGE;
  }

  size_t start_idx = rec_idx_[rid.slot_id_];
  char *start = page_->data() + start_idx;

  // already mark delete
  if (start[0] == 1) {
    return RC::RECORD_IS_DELETED;
  }

  return RC::SUCCESS;
}

inline bool RecordPage::get(Record &rec, const RecordId &rid)
{
  if (rec_idx_.empty() || rec_idx_.size() <= rid.slot_id_) {
    return false;
  }
  size_t start_idx = rec_idx_[rid.slot_id_];
  size_t end_idx = rid.slot_id_ == 0 ? PAGESIZE : rec_idx_[rid.slot_id_ - 1];
  char *start = page_->data() + start_idx;

  // already mark delete
  if (start[0] == 1) {
    rec.is_deleted_ = true;
    return false;
  }

  rec.data_.resize(end_idx - start_idx - 1);
  rec.is_deleted_ = false;
  rec.rid_ = rid;
  memcpy(rec.data_.data(), start + 1, rec.data_.size());
  return true;
}

inline bool RecordPage::get(RecordView &rec, const RecordId &rid)
{
  if (rec_idx_.empty() || rec_idx_.size() <= rid.slot_id_) {
    return false;
  }
  size_t start_idx = rec_idx_[rid.slot_id_];
  size_t end_idx = rid.slot_id_ == 0 ? PAGESIZE : rec_idx_[rid.slot_id_ - 1];
  char *start = page_->data() + start_idx;

  // already mark delete
  if (start[0] == 1) {
    rec.is_deleted_ = true;
    return false;
  }

  rec.data_ = BytesView{start + 1, page_->data() + end_idx};
  rec.is_deleted_ = false;
  rec.rid_ = rid;
  return true;
}
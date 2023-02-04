/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-04 15:55:55
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-04 20:42:53
 * @FilePath: /tadis/src/storage/io/record.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 * https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "common/bytes.hpp"
#include "storage/io/buffer_pool.hpp"
#include "storage/io/iodef.hpp"
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

  void init(Page *p)
  {
    assert(p);
    auto data = p->data();
    memcpy(reinterpret_cast<char *>(&free_addr_start_), data, sizeof(size_t));
    memcpy(reinterpret_cast<char *>(&free_addr_end_), data + sizeof(size_t), sizeof(size_t));
    memcpy(reinterpret_cast<char *>(&rec_idx_count_), data + 2 * sizeof(size_t), sizeof(size_t));
    rec_idx_.resize(rec_idx_count_);
    memcpy(rec_idx_.data(), data + 3 * sizeof(size_t), sizeof(size_t) * rec_idx_count_);
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

  void serlize2page()
  {
    auto data = page_->data();
    memcpy(data, reinterpret_cast<char *>(&free_addr_start_), sizeof(size_t));
    memcpy(data + sizeof(size_t), reinterpret_cast<char *>(&free_addr_end_), sizeof(size_t));
    memcpy(data + sizeof(size_t) * 2, reinterpret_cast<char *>(&rec_idx_count_), sizeof(size_t));
    memcpy(data + sizeof(size_t) * 3, rec_idx_.data(), rec_idx_.size());
  }

private:
  size_t remain_free_size() const
  {
    return free_addr_end_ - free_addr_start_;
  }

  size_t free_addr_start_;
  size_t free_addr_end_;
  size_t rec_idx_count_;
  std::vector<size_t> rec_idx_;

  Page *page_;
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
  // 此处第一个 +1是delete的标志位， -sizeof(size_t)是增加一个idx
  if (rec.data_.size() + 1 >= remain_free_size() - sizeof(size_t)) {
    return false;
  }
  size_t start_idx = free_addr_end_ - rec.data_.size();
  char *start = page_->data() + start_idx;

  // set delete mark
  start[0] = 0;

  // copy data
  memcpy(start + 1, rec.data_.data(), rec.data_.size());

  rid.page_id_ = page_->pid();
  rid.slot_id_ = rec_idx_.size() - 1;

  rec_idx_count_++;
  rec_idx_.push_back(start_idx);

  serlize2page();
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
  serlize2page();

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

  rec.data_.resize(start_idx - end_idx - 1);
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
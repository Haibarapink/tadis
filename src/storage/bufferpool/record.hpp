#pragma once

#include "common/logger.hpp"
#include "storage/bufferpool/buffer_pool.hpp"
#include "storage/storage_def.hpp"

#include <string>
#include <sstream>
#include <cassert>
#include <cstring>
#include <span>
#include <vector>

class Record;
class RecordView;
//  On disk :
//  |free_addr_start_|free_addr_end_|rec_idx_count_|...rec_idx...|...free...|rec3|  rec2  | rec1 |
//                               idx of record start
//
class RecordPage {
  friend class PageRecordScanner;
public:
  RecordPage() = default;
  RecordPage(Page *p)
  {
    init(p);
  }

  size_t record_count() const {
    return rec_idx_count_;
  }

  void init(Page *p)
  {
    assert(p);
    auto data = p->data();
    memcpy(reinterpret_cast<char *>(&free_addr_start_), data, sizeof(size_t));
    memcpy(reinterpret_cast<char *>(&free_addr_end_), data + sizeof(size_t), sizeof(size_t));
    memcpy(reinterpret_cast<char *>(&rec_idx_count_), data + 2 * sizeof(size_t), sizeof(size_t));
    if (free_addr_end_ == 0) {
      // first time
      free_addr_end_ = PAGE_SIZE;
      memcpy(data + sizeof(size_t), reinterpret_cast<char *>(&free_addr_end_), sizeof(size_t));
    }

    // clean
    if (!rec_idx_.empty()) {
      rec_idx_.clear();
    }

    for (size_t i = 0; i < rec_idx_count_; ++i) {
      size_t idx{0};
      memcpy(reinterpret_cast<char *>(&idx), data + (3 + i) * sizeof(size_t), sizeof(size_t));
      rec_idx_.push_back(idx);
    }
    page_ = p;
  }

  bool insert(const Record &rec, RID &rid);
  bool remove(const RID &rid);
  bool get(Record &rec, const RID &rid);
  bool get(RecordView &rec, const RID &rid);

  /**
   * @brief
   *  判断该RID是否存在 record
   * @return
   *   返回 OUT_OF_RANGE 表示扫描到 Page的结尾，不存在。
   *   返回 RECORD_IS_DELETED 表示扫描到一个被标记delete的record，也是不存在
   *  否则返回 RC::SUCCESS
   */
  RC contain(const RID &rid);

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
  void write_head(const RID &rid, size_t idx)
  {
    auto data = page_->data();
    memcpy(data, reinterpret_cast<char *>(&free_addr_start_), sizeof(size_t));
    memcpy(data + sizeof(size_t), reinterpret_cast<char *>(&free_addr_end_), sizeof(size_t));
    memcpy(data + sizeof(size_t) * 2, reinterpret_cast<char *>(&rec_idx_count_), sizeof(size_t));

    LOG_DEBUG << "recordIdx offset : " << sizeof(size_t) * (3 + rid.slot_id_) << " idx : " << idx;

    memcpy(data + sizeof(size_t) * (3 + rid.slot_id_), reinterpret_cast<char *>(&idx), sizeof(size_t));
  }

private:
  //@brief has enough space to insert a record
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
  size_t free_addr_end_ = PAGE_SIZE;
  size_t rec_idx_count_ = 0;
  std::vector<size_t> rec_idx_;

  Page *page_ = nullptr;
};

//@brief on disk : |is_delete_|..bytes..|
class Record {
  friend class RecordPage;
public:
  Record() = default;
  Record(std::vector<char> bytes) : data_(std::move(bytes)) {}
  template <typename IteratorType>
  Record(IteratorType begin, IteratorType end)
  {
    append(begin, end);
  }

  void reset()
  {
    is_deleted_ = false;
    data_ = std::vector<char>();
    rid_.page_id_ = INVALID_ID;
    rid_.slot_id_ = INVALID_ID;
  }

  // a record
  bool is_deleted() const
  {
    return is_deleted_;
  }

  auto rid()
  {
    return rid_;
  }

  std::vector<char> &bytes()
  {
    return data_;
  }

  template <typename IteratorType>
  void append(IteratorType begin, IteratorType end)
  {
    for (; begin != end; begin++) {
      data_.push_back(*begin);
    }
  }

  std::string_view to_string_view()
  {
    return std::string_view{data_.data(), data_.size()};
  }

  std::string to_string()
  {
    return std::string{data_.data(), data_.size()};
  }

private:
  bool is_deleted_ = false;
  RID rid_;
  std::vector<char> data_;
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

  std::span<char> &data()
  {
    return data_;
  }

private:
  bool is_deleted_ = false;
  RID rid_;
  std::span<char> data_;
};

// @brief scan all records in a page
class PageRecordScanner {
public:
  friend class RecordScanner;

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
  RC next(RecordType &rec, RID &rid)
  {
    while (true) {
      auto rc = rp_->contain(next_rid_);
      rp_->get(rec, next_rid_);
      if (rc == RC::OUT_OF_RANGE) {
        return rc;
      } else if (rc == RC::SUCCESS) {
        rid = next_rid_;
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
  RID next_rid_;
};

//@brief scan all records in all pages
class RecordScanner {
public:
  RecordScanner() = default;
  RecordScanner(BufferPool* bfp) {
    init(bfp);
  }
  ~RecordScanner()
  {
    bfp_->unpin(this->page_scanner_.next_rid_.page_id_, false);
  }

  RC init(BufferPool *bfp)
  {
    bfp_ = bfp;
    RID rid;
    rid.page_id_ = 1;
    rid.slot_id_ = 0;
    auto rc = bfp_->contain(1);
    auto page = bfp_->fetch(1);
    if (!page && rc == RC::SUCCESS) {
      return RC::INTERNAL_ERROR;
    }
    rp_.init(page);
    page_scanner_.init(&rp_);
    return RC::SUCCESS;
  }

  bool has_next()
  {
    bool cur_page_has_next = page_scanner_.has_next();
    if (!cur_page_has_next) {
      // fetch next
      PageId next_id = INVALID_ID;
      bool contain = bfp_->next_pid_after(page_scanner_.next_rid_.page_id_, next_id);
      bfp_->unpin(page_scanner_.next_rid_.page_id_, false);
      if (!contain) {
        return false;
      }
      // update
      page_scanner_.next_rid_.page_id_ = next_id;
      page_scanner_.next_rid_.slot_id_ = 0;
      page_scanner_.rp_ = nullptr;  // flag page is nullptr
      // we get next page
      auto next_page = bfp_->fetch(next_id);
      if (next_page == nullptr) {
        LOG_WARN << "fetch next page fail, still return true, and set page_scanner_.rp_ to nullptr";
        return true;
      }
      LOG_DEBUG << "fetch next page " << page_scanner_.next_rid_.to_string();
      rp_.init(next_page);
      page_scanner_.rp_ = &rp_;
      return page_scanner_.has_next();
    }
    return true;
  }

  template <typename RecordType>
  RC next(RecordType &rec, RID &rid)
  {
    if (has_next()) {
      // check page scanner
      if (page_scanner_.rp_ == nullptr) {
        auto next_page = bfp_->fetch(page_scanner_.next_rid_.page_id_);
        if (next_page == nullptr) {
          LOG_WARN << "can't fetch next page";
          return RC::INTERNAL_ERROR;
        }
        rp_.init(next_page);
        page_scanner_.rp_ = &rp_;
      }

      return page_scanner_.next(rec, rid);
    }
    return RC::OUT_OF_RANGE;
  }

private:
  PageRecordScanner page_scanner_;
  RecordPage rp_;
  BufferPool *bfp_;
};

inline bool RecordPage::insert(const Record &rec, RID &rid)
{
  if (!enough(rec.data_.size())) {
    return false;
  }

  size_t start_idx = free_addr_end_ - rec.data_.size() - 1;
  char *start = page_->data() + start_idx;

  // set delete mark
  start[0] = 0;

  // copy bytes
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

inline bool RecordPage::remove(const RID &rid)
{
  if (rec_idx_.empty() || rec_idx_.size() <= rid.slot_id_) {
    return false;
  }
  size_t start_idx = rec_idx_[rid.slot_id_];
  char *start = page_->data() + start_idx;
  start[0] = 1;

  return true;
}

inline RC RecordPage::contain(const RID &rid)
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

inline bool RecordPage::get(Record &rec, const RID &rid)
{
  if (rec_idx_.empty() || rec_idx_.size() <= rid.slot_id_) {
    return false;
  }
  size_t start_idx = rec_idx_[rid.slot_id_];
  size_t end_idx = rid.slot_id_ == 0 ? PAGE_SIZE : rec_idx_[rid.slot_id_ - 1];
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

inline bool RecordPage::get(RecordView &rec, const RID &rid)
{
  if (rec_idx_.empty() || rec_idx_.size() <= rid.slot_id_) {
    return false;
  }
  size_t start_idx = rec_idx_[rid.slot_id_];
  size_t end_idx = rid.slot_id_ == 0 ? PAGE_SIZE : rec_idx_[rid.slot_id_ - 1];
  char *start = page_->data() + start_idx;

  // already mark delete
  if (start[0] == 1) {
    rec.is_deleted_ = true;
    return false;
  }

  rec.data_ = std::span<char>{start + 1, page_->data() + end_idx};
  rec.is_deleted_ = false;
  rec.rid_ = rid;
  return true;
}
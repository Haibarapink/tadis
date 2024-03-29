#pragma once

#include "common/rc.hpp"
#include "common/bitmap.hpp"
#include "common/logger.hpp"
#include "common/noncopyable.hpp"
#include "storage/bufferpool/disk_manager.hpp"
#include "storage/storage_def.hpp"
#include "storage/bufferpool/replacer.hpp"

#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <string.h>


class Page;
class BufferPool;

// 第一个page记录元信息
// bitmap 记录empty page
class BFPHeaderPage {
public:
  // 实际文件中使用的block数量,不包含被删除的 block
  size_t page_num_ = 1;

  // 实际文件中使用的block数量,包含被删除的 block
  size_t phy_num_ = 1;
  std::vector<char> bitmap_;

  // 返回 true则表明这个数据库文件还没初始化
  bool init(Page *);

  void write2page(Page *);

  BitMap bitmap()
  {
    return BitMap{std::string_view{bitmap_.data(), bitmap_.size()}};
  }

  static size_t max_page_count()
  {
    return BFP_MAX_PAGE_NUM;
  }
};

class Page {
  friend class BufferPool;
  friend class BufferPoolTest;
public:

  Page() = default;
  ~Page() = default;

  Page(const Page &p)
  {
    copy(p);
  }

  Page &operator=(const Page &p)
  {
    copy(p);
    return *this;
  }

  Page(Page &&p)
  {
    copy(p);
    p.clear_all();
  }

  Page &operator=(Page &&p)
  {
    copy(p);
    p.clear_all();
    return *this;
  }

  void copy(const Page &p)
  {
    memcpy(this->buffer_.data(), p.buffer_.data(), PAGE_SIZE);
    this->dirty_ = p.dirty_;
    this->pid_ = p.pid_;
    this->pin_count_ = p.pin_count_;
  }

  void set_dirty(bool is_dirty)
  {
    dirty_ = is_dirty;
  }

  bool is_dirty() const
  {
    return dirty_;
  }

  char *data()
  {
    return buffer_.data();
  }

  size_t size() const
  {
    return buffer_.size();
  }

  void clear_data()
  {
    bzero(buffer_.data(), buffer_.size());
  }

  void clear_all()
  {
    clear_data();
    pid_ = INVALID_ID;
    pin_count_ = 0;
    dirty_ = false;
  }

  PageId pid() const
  {
    return pid_;
  }

private:
  std::array<char, PAGE_SIZE> buffer_;
  PageId pid_ = INVALID_ID;
  size_t pin_count_ = 0;
  bool dirty_ = false;
};

class BufferPool : public noncopyable {
  friend class BufferPoolTest;
public:
  BufferPool(std::string_view db_filename, size_t bfp_size) : replacer_(bfp_size), disk_(db_filename)
  {
    for (size_t i = 0; i < bfp_size; ++i) {
      pages_.emplace_back(new Page{});
      pages_.back()->clear_all();
      free_list_.emplace_back(i);
    }
    init();
  }

  BufferPool(std::string_view db_filename) : BufferPool(db_filename, 32)
  {}

  ~BufferPool()
  {
    if (!closed_) {
      close();
    }
  }

  void close()
  {
    if (!closed_) {
      auto page = fetch(0);
      if (page == nullptr) {
        return;
      }
      head_.write2page(page);
      unpin(page->pid_, true);
      flush_all_page();
      disk_.close();
      closed_ = true;
    }
  }

  void open()
  {
    assert(false && "not implement");
  }

  PageId current_pid()
  {
    return disk_.current_pid();
  }

  //@brief id is a valid page id
  //@return RC::OUT_OF_RANGE , RC::DELETED , RC::SUCCESS
  RC contain(PageId id)
  {
    if (head_.bitmap_.size() * 8 <= id) {
      return RC::OUT_OF_RANGE;
    }
    auto bitmap = head_.bitmap();
    bool contain = bitmap.get(id);
    return contain ? RC::SUCCESS : RC::PAGE_IS_DELETED;
  }

  //@brief get next PageId after current_pid
  bool next_pid_after(PageId current_pid, PageId &next_id)
  {
    auto bitmap = head_.bitmap();
    bool contain = bitmap.first_after(true, current_pid, next_id);
    return contain;
  }

  Page *fetch(PageId id)
  {
    if (auto iter = dir_.find(id); iter != dir_.end()) {
      Page *res = pages_[iter->second].get();
      res->pin_count_++;
      replacer_.remove(iter->second);
      LOG_DEBUG << "find frame " << iter->second << " in dir_";
      return res;
    }

    size_t frame_id = 0;
    if (!victim(frame_id)) {
      LOG_DEBUG << "victim frame fail";
      return nullptr;
    }

    LOG_DEBUG << "fetch frame " << frame_id << " by function 'victim'";
    Page *page = pages_[frame_id].get();
    change_correspondence(page, id, frame_id);
    disk_.read_page(id, page->data());
    page->pin_count_ = 1;
    page->pid_ = id;
    page->dirty_ = false;
    return page;
  }

  Page *new_page(PageId &id)
  {
    Page *res = nullptr;

    auto bitmap = head_.bitmap();
    PageId idx = INVALID_ID;
    if (auto ok = bitmap.first(false, idx); !ok || (ok && idx >= disk_.current_pid())) {
      idx = disk_.next_page_id();
      if (idx > BFPHeaderPage::max_page_count()) {
        LOG_DEBUG << "page eof";
        return nullptr;
      }
    }

    // 获取一个内存Page
    size_t frame_id = 0;
    if (!victim(frame_id)) {
      LOG_DEBUG << "victim frame fail";
      return nullptr;
    }

    while (head_.bitmap_.size() * 8 <= idx) {
      head_.bitmap_.push_back('\0');
      bitmap = head_.bitmap();
    }

    bitmap.set(idx, true);

    Page *page = pages_[frame_id].get();

    change_correspondence(page, idx, frame_id);

    head_.page_num_++;
    head_.phy_num_++;
    res = page;
    id = idx;

    res->pid_ = idx;
    res->dirty_ = true;
    res->pin_count_ = 1;

    return res;
  }

  void unpin(PageId id, bool is_dirty)
  {
    if (auto iter = dir_.find(id); iter != dir_.end()) {
      auto page = pages_[iter->second].get();
      bool dirty = page->is_dirty() || is_dirty;
      page->dirty_ = dirty;
      page->pin_count_--;
      if (page->pin_count_ == 0) {
        replacer_.put(iter->second);
      }
    }
  }

  void flush_page(PageId id)
  {
    if (auto iter = dir_.find(id); iter != dir_.end()) {
      auto page = pages_[iter->second].get();
      disk_.write_page(id, page->data());
    }
  }

  void delete_page(PageId id)
  {
    if (id >= head_.phy_num_) {
      LOG_WARN << "page " << id << " is not existed"
               << ", head_.phy_num_ is " << head_.phy_num_;
      return;
    }

    if (auto iter = dir_.find(id); iter != dir_.end()) {
      auto page = pages_[iter->second].get();
      auto frame_id = iter->second;
      auto bitmap = head_.bitmap();

      bitmap.set(id, false);
      page->clear_all();
      free_list_.push_front(frame_id);

      dir_.erase(iter);

      head_.page_num_--;

      auto head_page = fetch(0);
      if (head_page == nullptr) {
        LOG_WARN << "fetch head page fail";
        return;
      }

      head_.write2page(head_page);
      unpin(head_page->pid_, true);
    } else {
      auto bitmap = head_.bitmap();
      bitmap.set(id, false);
      head_.page_num_--;

      auto head_page = fetch(0);
      if (head_page == nullptr) {
        LOG_WARN << "fetch head page fail";
        return;
      }

      head_.write2page(head_page);
      unpin(head_page->pid_, true);
    }
  }

  void flush_all_page()
  {
    for (auto &&iter : dir_) {
      auto page = pages_[iter.second].get();
      if (page->is_dirty())
        disk_.write_page(iter.first, page->data());
    }
  }

private:
  // Init head page
  void init()
  {
    // head page
    auto page = fetch(0);
    if (page == nullptr) {
      LOG_WARN << "init fail!";
      return;
    }

    bool is_dirty = head_.init(page);

    unpin(0, is_dirty);
    if (is_dirty) {
      flush_page(0);
    }

    disk_.set_next_page_id(head_.phy_num_);
  }

  bool victim(size_t &frame_id)
  {
    if (!free_list_.empty()) {
      frame_id = free_list_.back();
      free_list_.pop_back();
      LOG_DEBUG << "freelist victim frameid : " << frame_id;
      return true;
    }

    bool res = replacer_.victim(frame_id);
    LOG_DEBUG << "replacer victim frameid : " << frame_id;
    return res;
  }

  void change_correspondence(Page *p, PageId &pid, size_t &fid)
  {
    if (p->is_dirty()) {
      flush_page(p->pid_);
    }
    dir_.erase(p->pid_);
    p->clear_all();

    p->pid_ = pid;

    dir_[pid] = fid;
    LOG_DEBUG << "pid " << pid << "<->"
              << "fid " << fid;
  }

  // 'size_t' is the frame pointer whitch point to pages_' item.
  std::unordered_map<PageId, size_t> dir_;
  std::list<size_t> free_list_;

  std::vector<std::unique_ptr<Page>> pages_;
  LruReplacer<size_t> replacer_;
  DiskManager disk_;

  BFPHeaderPage head_;

  bool closed_ = false;
};

inline bool BFPHeaderPage::init(Page *page)
{
  assert(page);
  auto data = page->data();
  memcpy(&page_num_, data, sizeof(size_t));
  memcpy(&phy_num_, data + sizeof(size_t), sizeof(size_t));

  // 刚刚初始化的文件
  if (page_num_ == 0) {
    page_num_ = 1;
    phy_num_ = 1;
    // init
    bitmap_ = std::vector<char>(1, '\0');
    auto bm = this->bitmap();
    bm.set(0, true);
    write2page(page);
    return true;
  }

  size_t bitmap_size = 0;
  if (phy_num_ > 8) {
    bitmap_size = page_num_ / 8 + (page_num_ % 8 == 0 ? 0 : 1);
  } else {
    bitmap_size = 1;
  }

  bitmap_ = std::vector<char>(bitmap_size, '\0');
  memcpy(bitmap_.data(), data + 2 * sizeof(size_t), bitmap_size);

  return false;
}

inline void BFPHeaderPage::write2page(Page *page)
{
  assert(page);
  memcpy(page->data(), reinterpret_cast<char *>(&this->page_num_), sizeof(size_t));
  memcpy(page->data() + sizeof(size_t), reinterpret_cast<char *>(&this->phy_num_), sizeof(size_t));
  memcpy(page->data() + 2 * sizeof(size_t), bitmap_.data(), bitmap_.size());
  page->set_dirty(true);
}
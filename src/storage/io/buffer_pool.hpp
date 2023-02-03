/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-02 12:49:27
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-03 15:05:10
 * @FilePath: /tadis/src/storage/kv/bufferpool.hpp
 * @Description: buffer pool
 */
#pragma once

#include <array>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <strings.h>
#include <unordered_map>
#include <vector>

#include "common/rc.hpp"
#include "common/bitmap.hpp"
#include "common/logger.hpp"
#include "storage/io/disk.hpp"
#include "storage/io/iodef.hpp"
#include "storage/io/replacer.hpp"

class Page;
class BufferPool;

constexpr size_t INVALID_ID = SIZE_MAX;

// 第一个page记录元信息
// bitmap 记录empty page
class HeadPage {
public:
  // 实际文件中使用的block数量,不包含被删除的 block
  size_t page_num_ = 1;

  // 实际文件中使用的block数量,包含被删除的 block
  size_t phy_num_ = 1;
  std::vector<char> bitmap_;

  // 返回 true则表明这个数据库文件还没初始化
  bool init(Page *);

  void serlize2page(Page *);

  BitMap bitmap()
  {
    return BitMap{std::string_view{bitmap_.data(), bitmap_.size()}};
  }

  static size_t max_page_count()
  {
    return (PAGESIZE - sizeof(page_num_)) * 8;
  }
};

class Page {
public:
  friend class BufferPool;

  Page() = default;
  ~Page() = default;

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

private:
  std::array<char, PAGESIZE> buffer_;
  PageId pid_ = INVALID_ID;
  size_t pin_count_ = 0;
  bool dirty_ = false;
};

class BufferPool {
public:
  BufferPool(std::string_view db_filename) : disk_(db_filename)
  {
    init();
  }

  void close()
  {
    auto page = fetch(0);
    if (page == nullptr) {
      return;
    }
    head_.serlize2page(page);
    unpin(page->pid_, true);
    flush_page(page->pid_);
  }

  Page *fetch(PageId id)
  {
    if (auto iter = dir_.find(id); iter != dir_.end()) {
      Page *res = pages_[iter->second].get();
      res->pin_count_++;
      replacer_.remove(iter->second);
      return res;
    }
    size_t frame_id = 0;
    if (!victim(frame_id)) {
      LOG_DEBUG << "victim frame fail";
      return nullptr;
    }

    Page *page = pages_[frame_id].get();
    page->clear_all();
    change_correspondence(page, id, frame_id);
    disk_.read_page(id, page->data());
    replacer_.remove(frame_id);
    page->pin_count_ = 1;
    return page;
  }

  Page *new_page(PageId &id)
  {
    Page *res = nullptr;

    auto bitmap = head_.bitmap();
    PageId idx = INVALID_ID;

    if (bitmap.first(false, idx)) {
    } else {
      idx = disk_.next_page_id();
      if (idx > HeadPage::max_page_count()) {
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

    // 判断是否越界
    if (idx / 8 > head_.bitmap_.size()) {
      head_.bitmap_.push_back('\0');
    }

    bitmap.set(idx, true);

    Page *page = pages_[frame_id].get();
    page->clear_all();
    change_correspondence(page, id, frame_id);

    head_.page_num_++;
    res = page;
    res->dirty_ = true;
    res->pin_count_ = 1;
    res->pid_ = idx;

    return res;
  }

  void unpin(PageId id, bool is_dirty)
  {
    if (auto iter = dir_.find(id); iter != dir_.end()) {
      auto page = pages_[iter->second].get();
      page->dirty_ = true;
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
      head_.serlize2page(head_page);
      unpin(head_page->pid_, true);
    }
  }

  void flush_all_page()
  {
    for (auto &&iter : dir_) {
      auto page = pages_[iter.second].get();
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

    disk_.set_next_page_id(head_.phy_num_);
  }

  bool victim(size_t &frame_id)
  {
    if (!free_list_.empty()) {
      frame_id = free_list_.back();
      free_list_.pop_back();
      return true;
    }

    return replacer_.victim(frame_id);
  }

  void change_correspondence(Page *p, PageId &pid, size_t &fid)
  {
    if (p->is_dirty()) {
      flush_page(p->pid_);
    }
    dir_[pid] = fid;
  }

  // 'size_t' is the frame pointer whitch point to pages_' item.
  std::unordered_map<PageId, size_t> dir_;
  std::list<size_t> free_list_;

  std::vector<std::unique_ptr<Page>> pages_;
  LruReplacer<size_t> replacer_;
  DiskManager disk_;

  HeadPage head_;
};

inline bool HeadPage::init(Page *page)
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
    serlize2page(page);
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

inline void HeadPage::serlize2page(Page *page)
{
  assert(page);
  memcpy(page->data(), reinterpret_cast<char *>(&this->page_num_), sizeof(size_t));
  memcpy(page->data() + sizeof(size_t), reinterpret_cast<char *>(&this->phy_num_), sizeof(size_t));
  memcpy(page->data() + 2 * sizeof(size_t), bitmap_.data(), bitmap_.size());
  page->set_dirty(true);
}
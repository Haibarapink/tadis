/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-02 12:49:27
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-03 13:11:10
 * @FilePath: /tadis/src/storage/kv/bufferpool.hpp
 * @Description: buffer pool
 */
#pragma once

#include <array>
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

constexpr size_t INVALID_ID = 18446744073709551615;

// 第一个page记录元信息
// bitmap 记录empty page
class HeadPage {
public:
  size_t page_number_;
  std::vector<char> bitmap_;

  void init(Page *);

  BitMap bitmap()
  {}

  static size_t max_page_count()
  {
    return (PAGESIZE - sizeof(page_number_)) * 8;
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

    head_.page_number_++;
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

void HeadPage::init(Page *page)
{
  assert(page);
  auto data = page->data();
  memcpy(&page_number_, data, sizeof(size_t));
  size_t bitmap_size = page_number_ > 8 ? page_number_ / 8 : 1;
  bitmap_.resize(bitmap_size);
  memcpy(bitmap_.data(), data + sizeof(size_t), bitmap_size);
}
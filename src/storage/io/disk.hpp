/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-02 16:05:15
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-03 18:32:10
 * @FilePath: /tadis/src/storage/io/disk.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 * https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include "common/logger.hpp"
#include "storage/io/iodef.hpp"
#include "storage/io/fileop.hpp"
#include "storage/io/iodef.hpp"

#include <algorithm>
#include <mutex>
#include <cassert>
#include <fstream>
#include <ios>
#include <strings.h>

class DiskManager {
public:
  DiskManager(std::string_view filename, PageId next_start_id) : db_filename_(filename), next_page_id_(next_start_id)
  {
    db_io_.open(filename.data(), std::ios::binary | std::ios::in | std::ios::app | std::ios::out);
    assert(db_io_.is_open());
  }

  DiskManager(std::string_view filename) : DiskManager(filename, 1)
  {}

  void set_next_page_id(PageId next_page_id)
  {
    next_page_id_ = next_page_id;
  }

  void read_page(PageId id, char *dst);
  void write_page(PageId id, char *src);

  void shutdown();

  PageId next_page_id()
  {
    return next_page_id_++;
  }

  PageId cur_page_id()
  {
    return next_page_id_;
  }

private:
  std::mutex mutex_;
  std::string db_filename_;
  std::fstream db_io_;

  PageId next_page_id_ = 1;  // default 1
};

inline void DiskManager::read_page(PageId id, char *dst)
{
  std::unique_lock<std::mutex> lock{mutex_};
  size_t file_size = filesize(std::string_view{db_filename_.data(), db_filename_.size()});
  auto offset = id * PAGESIZE;
  if (offset > file_size) {
    LOG_DEBUG << "page is not existed";
  } else {
    db_io_.seekp(offset);
    db_io_.read(dst, PAGESIZE);
    if (db_io_.bad()) {
      LOG_DEBUG << "io error while reading";
      return;
    }
  }
}

inline void DiskManager::write_page(PageId id, char *src)
{
  std::unique_lock<std::mutex> lock{mutex_};
  auto offset = id * PAGESIZE;
  db_io_.seekp(offset);
  db_io_.write(src, PAGESIZE);
  if (db_io_.bad()) {
    LOG_DEBUG << "io error while writing";
    return;
  }
  db_io_.flush();
}

inline void DiskManager::shutdown()
{
  std::unique_lock<std::mutex> lock{mutex_};
  db_io_.close();
}
#pragma once

#include "common/logger.hpp"
#include "storage/storage_def.hpp"

#include <algorithm>
#include <mutex>
#include <cassert>
#include <fstream>
#include <ios>
#include <cstdio>
#include <filesystem>
#include <string.h>

class DiskManager {
public:
 DiskManager(std::string_view filename, PageId next_start_id) : db_filename_(filename), next_page_id_(next_start_id)
 {

   std::filesystem::path file_path(db_filename_.data());
   if (std::filesystem::exists(file_path) && std::filesystem::is_regular_file(file_path)) {
     db_io_ = fopen(filename.data(), "rb+");
   } else {
     db_io_ = fopen(filename.data(), "wb+");
   }

   assert(db_io_);
 }

 DiskManager(std::string_view filename) : DiskManager(filename, 1)
 {}

 void set_next_page_id(PageId next_page_id)
 {
   next_page_id_ = next_page_id;
 }

 void read_page(PageId id, char *dst);
 void write_page(PageId id, char *src);

 void close();

 PageId next_page_id()
 {
   return next_page_id_++;
 }

 PageId current_pid()
 {
   return next_page_id_ - 1;
 }

private:
 std::mutex mutex_;
 std::string db_filename_;

 FILE *db_io_;

 PageId next_page_id_ = 1;  // default 1
};

inline void DiskManager::read_page(PageId id, char *dst)
{
 std::unique_lock<std::mutex> lock{mutex_};
 size_t file_size = std::filesystem::file_size(db_filename_.data());
 auto offset = id * PAGE_SIZE;

 if (offset > file_size) {
   LOG_DEBUG << "page is not existed";
 } else {
   fseek(db_io_, offset, SEEK_SET);
   auto count = fread(dst, PAGE_SIZE, 1, db_io_);
   if (count != 1) {
     LOG_DEBUG << "io error while reading";
     return;
   }
 }
}

inline void DiskManager::write_page(PageId id, char *src)
{
 std::unique_lock<std::mutex> lock{mutex_};
 auto offset = id * PAGE_SIZE;
 fseek(db_io_, offset, SEEK_SET);
 auto count = fwrite(src, PAGE_SIZE, 1, db_io_);
 if (count != 1) {
   LOG_DEBUG << "io error while writing";
   return;
 }
 fflush(db_io_);
}

inline void DiskManager::close()
{
 std::unique_lock<std::mutex> lock{mutex_};
 if (db_io_)
   fclose(db_io_);
}
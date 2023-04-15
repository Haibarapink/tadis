#pragma once

#include "storage/bufferpool/record.hpp"
#include "storage/storage_def.hpp"

class TableHeap {
public:
  TableHeap(const std::string& name, size_t pool_size = 16) : buffer_pool_(new BufferPool(name, pool_size)) {}

  ~TableHeap() {
//    close();
  }

  auto scanner() -> RecordScanner* {

    return new RecordScanner{buffer_pool_.get()};
  }

  bool insert(Record record) {
     auto pid = buffer_pool_->current_pid();
     Page* page = nullptr;
     if (pid == INVALID_ID || buffer_pool_->current_pid() == 0) {
        page = buffer_pool_->new_page(pid);
        if (page == nullptr) {
          return false;
        }
     } else {
        page = buffer_pool_->fetch(pid);
     }
     RecordPage record_page{page};
     RID rid;
     auto ok = record_page.insert(std::move(record), rid);
     if (!ok) {
         buffer_pool_->unpin(pid, false);
         page = buffer_pool_->new_page(pid);
         if (page == nullptr) {
            return false;
         }
         record_page = RecordPage{page};
         ok = record_page.insert(std::move(record), rid);
     }
     buffer_pool_->unpin(pid, true);
     return ok;
  }

  bool remove(RID& rid) {
    auto page = buffer_pool_->fetch(rid.page_id_);
    if (page == nullptr) {
      return false;
    }
    RecordPage record_page{page};
    bool ok = record_page.remove(rid);
    // TODO free page
    if (record_page.record_count() == 0) {
      // buffer pool free page
    }
    buffer_pool_->unpin(rid.page_id_, true);
    return ok;
  }

  void close() {
    buffer_pool_->close();
  }

  auto buffer_pool() -> BufferPool* {
    return buffer_pool_.get();
  }

private:
  std::unique_ptr<BufferPool> buffer_pool_;
};
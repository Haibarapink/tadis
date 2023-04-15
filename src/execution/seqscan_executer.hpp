#pragma once

#include "storage/bufferpool/record.hpp"
#include "execution/execution.hpp"

class SeqScanExectuer : public Executer  {
public:
  SeqScanExectuer(RecordScanner* scanner, Schema* schema): scanner_(scanner) {
    schema_ = std::unique_ptr<Schema>{schema};
  }

   RC next(Tuple* tuple) override {
     assert(schema_ && scanner_);
     Record record;
     RID rid;
     auto ok = scanner_->next(record, rid);
     if (rc_success(ok)) {
       tuple->data_ = std::move(record.bytes());
       tuple->rid_ = rid;
     }
     return ok;
  }

private:
    std::unique_ptr<RecordScanner> scanner_;
};
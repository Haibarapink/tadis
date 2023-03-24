#pragma once

#include "operator/operator.hpp"

class DeleteOp : public Operator {
public:
private:
  RecordScanner scanner_;
};

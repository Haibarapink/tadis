#pragma once

#include "stage/stage.hpp"
#include "storage/tuple.hpp"
#include <functional>
#include <vector>

class ResultStage : public Stage {
public:
  using ResultCallBack = std::function<void(std::vector<Tuple> &tuples, RC rc_res)>;

  static ResultCallBack defaultCb;

  void init(std::vector<Tuple> tp, RC rc, ResultCallBack cb = defaultCb)
  {
    cb_ = std::move(cb);
    tuples_ = std::move(tp);
    rc_ = rc;
  }

  RC handle_event()
  {
    if (cb_)
      cb_(tuples_, rc_);
    return RC::SUCCESS;
  }

private:
  ResultCallBack cb_;
  std::vector<Tuple> tuples_;
  RC rc_;
};

ResultStage::ResultCallBack ResultStage::defaultCb = nullptr;
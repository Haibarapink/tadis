#pragma once

#include <cassert>
#include <utility>
#include <variant>
#include <string>

template <typename T, typename Error>
class Expect {
public:
  Expect() = default;
  ~Expect() = default;

  bool ok() const
  {
    return std::holds_alternative<T>(inter_);
  }

  Expect(T t) : inter_(std::move(t))
  {}
  Expect(Error e, std::string msg = "") : inter_(e), msg_(msg)
  {}

  Expect(const Expect &other) : inter_(other.inter_)
  {}

  Expect &operator=(const Expect &other)
  {
    inter_ = other.inter_;
    return *this;
  }

  Expect(Expect &&other) : inter_(std::move(other.inter_))
  {}

  Expect &operator=(Expect &&other)
  {
    inter_ = std::move(other.inter_);
    return *this;
  }

  Expect &operator=(T t)
  {
    inter_ = std::move(t);
    return *this;
  }

  Expect &operator=(Error e)
  {
    inter_ = std::move(e);
    return *this;
  }

  Error take_error()
  {
    if (!std::holds_alternative<Error>(inter_)) {
      assert(false);
    }
    return std::get<Error>(inter_);
  }

  const std::string msg() const
  {
    return msg_;
  }

  T take_T()
  {
    if (!std::holds_alternative<T>(inter_)) {
      assert(false);
    }
    T res = std::move(std::get<T>(inter_));
    return std::move(res);
  }

private:
  std::variant<T, Error> inter_;
  std::string msg_;
};

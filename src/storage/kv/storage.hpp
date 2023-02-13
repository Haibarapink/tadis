/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-02 19:34:49
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-15 00:23:59
 * @FilePath: /tadis/src/storage/storage.hpp
 */
#pragma once
#include "common/rc.hpp"
#include <span>
#include <vector>
#include <cstdint>

using Bytes = std::vector<char>;
using BytesView = std::span<char>;

template <typename T>
class TStorage : public noncopyable {
public:
  RC init(std::string_view path)
  {
    auto t = (T *)(this);
    return t->init(path);
  }

  RC get(const std::vector<char> &key, std::vector<char> &value)
  {
    auto t = (T *)(this);
    return t->get(key, value);
  }

  RC remove(const std::vector<char> &key)
  {
    auto t = (T *)(this);
    return t->remove(key);
  }

  RC flush()
  {
    auto t = (T *)(this);
    return t->flush();
  }

  RC close()
  {
    auto t = (T *)(this);
    return t->close();
  }
};

class Storage : public noncopyable {
public:
  virtual RC get(const std::vector<char> &key, std::vector<char> &value) = 0;
  virtual RC set(std::vector<char> key, std::vector<char> value) = 0;
  virtual RC remove(const std::vector<char> &key) = 0;
  virtual RC flush() = 0;
  virtual RC close() = 0;
};

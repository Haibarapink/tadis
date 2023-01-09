/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-02 19:34:49
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-09 23:22:57
 * @FilePath: /tadis/src/storage/storage.hpp
 */
#pragma once
#include "common/rc.hpp"
#include <boost/core/noncopyable.hpp>
#include <vector>
#include <cstdint>
#include <boost/noncopyable.hpp>
using Bytes = std::vector<uint8_t>;

template <typename T>
class TStorage : public boost::noncopyable {
public:
  RC init(const std::string &path)
  {
    auto t = (T *)(this);
    return t->init(path);
  }

  RC get(const std::vector<uint8_t> &key, std::vector<uint8_t> &value)
  {
    auto t = (T *)(this);
    return t->get(key, value);
  }

  RC remove(const std::vector<uint8_t> &key)
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

class Storage : public boost::noncopyable {
public:
  virtual RC get(const std::vector<uint8_t> &key, std::vector<uint8_t> &value) = 0;
  virtual RC set(std::vector<uint8_t> key, std::vector<uint8_t> value) = 0;
  virtual RC remove(const std::vector<uint8_t> &key) = 0;
  virtual RC flush() = 0;
  virtual RC close() = 0;
};

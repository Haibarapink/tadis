/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-09 16:16:48
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-02 16:37:13
 * @FilePath: /tadis/src/storage/memory.hpp
 * An in-memory storage
 */
#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <iterator>
#include <storage/kv/storage.hpp>

class InMemoryStorage : public TStorage<InMemoryStorage> {
public:
  class Iterator {
  public:
    explicit Iterator(std::map<Bytes, Bytes>::iterator iter) : p_(iter)
    {}
    Iterator &operator++()
    {
      p_++;
      return *this;
    }
    Iterator operator++(int)
    {
      auto res = *this;
      p_++;
      return res;
    }
    bool operator==(Iterator other)
    {
      return p_ == other.p_;
    }

    bool operator!=(Iterator other)
    {
      return p_ != other.p_;
    }
    auto operator*() const
    {
      return (*p_);
    }

  private:
    std::map<Bytes, Bytes>::iterator p_;
  };

public:
  InMemoryStorage()
  {}
  ~InMemoryStorage()
  {}

  RC init(const std::string &path)
  {
    name_ = path;
    return RC::SUCCESS;
  }

  RC remove(const std::vector<char> &key)
  {
    auto iter = kv_.find(key);
    if (iter == kv_.end()) {
      return RC::KEY_NOT_EXIST;
    }
    kv_.erase(iter);
    return RC::SUCCESS;
  }

  RC get(const std::vector<char> &key, std::vector<char> &value)
  {
    auto iter = kv_.find(key);
    if (iter == kv_.end()) {
      return RC::KEY_NOT_EXIST;
    }
    value = iter->second;
    return RC::SUCCESS;
  }

  RC set(std::vector<char> key, std::vector<char> value)
  {
    kv_.emplace(std::move(key), std::move(value));
    return RC::SUCCESS;
  }

  RC flush()
  {
    return RC::SUCCESS;
  }

  RC close()
  {
    return RC::SUCCESS;
  }

  Iterator begin()
  {
    return Iterator(kv_.begin());
  }

  Iterator end()
  {
    return Iterator(kv_.end());
  }

private:
  std::map<std::vector<char>, std::vector<char>> kv_;
  std::string name_;
};

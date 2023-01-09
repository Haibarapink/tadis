/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-09 16:16:48
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-09 23:17:51
 * @FilePath: /tadis/src/storage/memory.hpp
 * An in-memory storage
 */
#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <storage/storage.hpp>

class InMemoryStorage : public Storage {
public:
  InMemoryStorage()
  {}
  ~InMemoryStorage()
  {}

  RC remove(const std::vector<uint8_t> &key) override
  {
    auto iter = kv_.find(key);
    if (iter == kv_.end()) {
      return RC::KEY_NOT_EXIST;
    }
    kv_.erase(iter);
    return RC::SUCCESS;
  }

  RC get(const std::vector<uint8_t> &key, std::vector<uint8_t> &value) override
  {
    auto iter = kv_.find(key);
    if (iter == kv_.end()) {
      return RC::KEY_NOT_EXIST;
    }
    value = iter->second;
    return RC::SUCCESS;
  }

  RC set(std::vector<uint8_t> key, std::vector<uint8_t> value) override
  {
    kv_.emplace(std::move(key), std::move(value));
    return RC::SUCCESS;
  }

  RC flush() override
  {
    return RC::SUCCESS;
  }

  RC close() override
  {
    return RC::SUCCESS;
  }

private:
  std::map<std::vector<uint8_t>, std::vector<uint8_t>> kv_;
};
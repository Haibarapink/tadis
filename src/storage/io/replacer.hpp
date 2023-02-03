/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-02 13:15:29
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-03 17:00:41
 * @FilePath: /tadis/src/storage/io/replacer.hpp
 * @Description LRU replacer
 */
#include <cstddef>
#include <unordered_map>
#include <list>

constexpr size_t DEFAULT_LRU_CAPACITY = 16;

template <typename T>
class LruReplacer {
public:
  LruReplacer() : capacity_(DEFAULT_LRU_CAPACITY)
  {}

  LruReplacer(size_t cap) : capacity_(cap)
  {}

  void put(T t)
  {
    if (auto iter = dir_.find(t); iter != dir_.end()) {
      auto list_iter = iter->second;
      lru_list_.erase(list_iter);
      dir_.erase(iter);
    } else {
      if (capacity_ == lru_list_.size()) {
        auto &&back_value = lru_list_.back();
        dir_.erase(back_value);
        lru_list_.pop_back();
      }
    }
    lru_list_.push_front(t);
    dir_.emplace(std::move(t), lru_list_.begin());
  }

  // @brief If replacer contain t return true and update the t in replacer, else return false,
  bool touch(const T &t)
  {
    if (auto iter = dir_.find(t); iter != dir_.end()) {
      auto list_iter = iter->second;
      lru_list_.erase(list_iter);
      dir_[t] = lru_list_.begin();
      lru_list_.push_front(t);
      return true;
    }
    return false;
  }

  void remove(const T &t)
  {
    if (auto iter = dir_.find(t); iter != dir_.end()) {
      lru_list_.erase(iter->second);
      dir_.erase(iter);
    }
  }

  bool victim(T &t)
  {
    if (!lru_list_.empty()) {
      t = lru_list_.back();
      dir_.erase(t);
      lru_list_.pop_back();
      return true;
    }
    return false;
  }

private:
  std::list<T> lru_list_;
  std::unordered_map<T, typename std::list<T>::iterator> dir_;

  size_t capacity_;
};
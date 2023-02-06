/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-03 10:33:05
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-06 14:47:49
 * @FilePath: /tadis/src/common/bitmap.hpp
 * @Description: Bitmap
 */
#pragma once

#include <cassert>
#include <string>
#include <string_view>

class BitMap {
public:
  BitMap(std::string_view view) : data_(view)
  {}

  std::string to_string()
  {
    std::string res;

    for (auto ch : data_) {
      for (auto i = 0; i < 8; ++i) {
        bool bit = ch & (1 << i);
        res.push_back(bit ? '1' : '0');
      }
      res.push_back(' ');
    }

    return res;
  }

  // 获得第一个和参数bit相同的bit， 如果找不到就返回false,否则就返回true，结果保存在参数idx上
  bool first(bool bit, size_t &idx)
  {
    for (size_t i = 0; i < data_.size() * 8; ++i) {
      if (get(i) == bit) {
        idx = i;
        return true;
      }
    }
    return false;
  }

  // 获得第一个在cur_idx之后的和参数相同的bit的位置
  bool first_after(bool bit, size_t cur_idx, size_t &idx)
  {
    if (this->data_.size() * 8 <= cur_idx) {
      return false;
    }
    for (size_t i = cur_idx + 1; i < data_.size() * 8; ++i) {
      if (get(i) == bit) {
        idx = i;
        return true;
      }
    }
    return false;
  }

  bool get(size_t idx)
  {
    auto index = idx / 8;
    auto offset = idx % 8;
    assert(index < data_.size());
    return data_[index] & (1 << offset);
  }

  void set(size_t idx, bool bit)
  {
    auto index = idx / 8;
    auto offset = idx % 8;
    assert(index < data_.size());
    auto &&ch = const_cast<char &>(data_[index]);

    if (bit == true) {
      ch |= (1 << offset);
    } else {
      ch &= ~(1 << offset);
    }
  }

  void set2one(size_t idx)
  {
    set(idx, true);
  }

  void set2zero(size_t idx)
  {
    set(idx, false);
  }

private:
  std::string_view data_;
};
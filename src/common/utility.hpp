/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-09 23:08:24
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-31 19:28:19
 * @FilePath: /tadis/src/common/unit.hpp
 */
#pragma once

#include <unordered_set>
#include <cstdint>
#include <vector>
#include <string>
#include <string_view>

template <typename StringType>
inline std::vector<uint8_t> string2vector(const StringType &s)
{
  std::vector<uint8_t> v;
  for (auto ch : s) {
    v.push_back(static_cast<uint8_t>(ch));
  }
  return v;
}

inline std::string vector2string(std::vector<uint8_t> &v)
{
  std::string s;
  for (auto byte : v) {
    s.push_back(static_cast<char>(byte));
  }
  return s;
}

inline std::string make_meta_filename(std::string_view dir, std::string_view table_name)
{
  std::string res;
  res.append(dir);
  res.append("/");
  res.append("table_");
  res.append(table_name);
  res.append("_meta.json");
  return res;
}

inline std::string make_data_filename(std::string_view dir, std::string_view table_name)
{
  std::string res;
  res.append(dir);
  res.append("/");
  res.append("table_");
  res.append(table_name);
  res.append("_data.db");
  return res;
}

// Spit the input string by setting the split charactors.
class Spliter {
public:
  Spliter() : p_{0}
  {}

  Spliter(std::string_view input) : input_(input), p_(0)
  {}

  void init(std::string_view input)
  {
    input_ = input;
    p_ = 0;
  }

  /**
   *@brief Add the charactor which splitted the input.
   */
  void add_split_ch(char ch)
  {
    split_chars_.insert(ch);
  }

  std::string_view next()
  {
    auto start = input_.data() + p_;
    auto end = p_;
    for (; end < input_.size(); ++end) {
      char ch = input_[end];
      if (split_chars_.count(ch) > 0) {
        auto res = std::string_view{start, end - p_};
        p_ = end + 1;
        return res;
      }
    }
    auto res = std::string_view{start, end - p_};
    p_ = end;
    return res;
  }

  bool eof()
  {
    return p_ >= input_.size();
  }

private:
  std::string_view input_;
  std::unordered_set<char> split_chars_;
  size_t p_;
};
#pragma once

#include <string_view>
#include <cstring>

template <typename NumType>
inline int cmp_num(NumType l , NumType r)
{
  if (l > r) {
    return 1;
  }
  return l == r ? 0 : -1;
}

inline int cmp_str(std::string_view l, std::string_view r)
{
  size_t l_len = l.size(), r_len = r.size();
  int res = strncmp(l.data(), r.data(), std::min(l_len, r_len));
  if (res == 0) {
    if (l_len > r_len) {
      res = 1;
    } else if (l_len < r_len) {
      res = -1;
    }
  }
  return res;
}


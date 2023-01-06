#pragma once
#include <boost/filesystem/fstream.hpp>
#include <cstdint>
#include <istream>
#include <map>
#include <memory>
#include <string_view>
#include <cstdio>
#include <boost/filesystem.hpp>
#include <sys/types.h>
#include <vector>
#include "common/code.hpp"
#include "storage/storage.hpp"

// An example of storage, just a example!
// key and value split by '\n' in the files.

inline std::vector<uint8_t> str_to_vector(const std::string &str)
{
  std::vector<uint8_t> res;
  for (auto i : str) {
    res.emplace_back(static_cast<uint8_t>(i));
  }

  return res;
}

inline std::string_view vec_to_str(const std::vector<uint8_t> &vec)
{
  return std::string_view((char *)vec.data(), vec.size());
}

class RBTreeStore : public TStorage<RBTreeStore> {
public:
  RC init(std::string path)
  {
    path_ = std::move(path);
    return load();
  }

  RC get(const std::vector<uint8_t> &key, std::vector<uint8_t> &value)
  {
    RC rc = RC::SUCCESS;
    auto iter = kv_.find(key);
    if (iter == kv_.end()) {
      return RC::INTERNAL_ERROR;
    } else {
      value = iter->second;
    }
    return rc;
  }

  RC remove(const std::vector<uint8_t> &key)
  {
    auto iter = kv_.find(key);
    if (iter == kv_.end()) {
      return RC::INTERNAL_ERROR;
    }
    kv_.erase(iter);
    return RC::SUCCESS;
  }

  RC flush()
  {
    return RC::SUCCESS;
  }

  RC close()
  {
    kv_.clear();
    return RC::SUCCESS;
  }

private:
  RC load();

  std::string path_;
  std::map<Bytes, Bytes> kv_;
};

RC RBTreeStore::load()
{
  RC rc{RC::SUCCESS};
  FILE *f{fopen(path_.c_str(), "r")};
  if (f == nullptr) {
    return RC::INTERNAL_ERROR;
  }

  boost::filesystem::ifstream fs{path_};
  std::array<char, 4096> buf;
  std::pair<Bytes, Bytes> entry;
  bool is_key = true;
  while (fs.getline(&buf[0], 4096, '\n')) {
    std::vector<uint8_t> &tmp = is_key ? entry.first : entry.second;
    for (auto i = 0; i < buf.size(); ++i) {
      char ch = buf[i];
      if (ch == '\0') {
        break;
      }
      tmp.emplace_back(static_cast<uint8_t>(ch));
    }
    if (is_key) {
      is_key = false;
    } else {
      kv_.emplace(std::move(entry));
      is_key = true;
    }
  }
  return rc;
}
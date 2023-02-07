/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-13 23:46:59
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 03:58:34
 * @FilePath: /tadis/src/storage/io/readfile.hpp
 */
#pragma once
#include "common/bytes.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string_view>
#include <filesystem>

/**
 *@brief 一次读完全部文件
 */
inline Bytes std_readfile(std::string_view filename)
{
  FILE *f = fopen(filename.data(), "r+");
  auto fsize = std::filesystem::file_size(std::filesystem::path{filename});
  Bytes bytes(fsize);
  fread(bytes.data(), fsize, 1, f);
  fclose(f);
  return bytes;
}

/**
 *@brief 文件大小, 内部使用， 已经确定文件存在
 */
inline size_t filesize(std::string_view filename)
{
  auto path = std::filesystem::path{filename};
  assert(std::filesystem::is_regular_file(path));
  return static_cast<size_t>(std::filesystem::file_size(path));
}

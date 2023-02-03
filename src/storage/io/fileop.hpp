/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-13 23:46:59
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-02 16:35:28
 * @FilePath: /tadis/src/storage/io/readfile.hpp
 */
#pragma once
#include "common/bytes.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <iterator>
#include <string_view>
#include <boost/filesystem.hpp>

/**
 *@brief 一次读完全部文件
 */
inline Bytes std_readfile(std::string_view filename)
{
  std::ifstream ifs{filename.data(), std::ios::in | std::ios::binary | std::ios::ate};
  auto filesize = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  Bytes bytes;
  ifs.read(bytes.data(), filesize);
  ifs.close();
  return bytes;
}

/**
 *@brief 文件大小, 内部使用， 已经确定文件存在
 */
inline size_t filesize(std::string_view filename)
{
  auto path = boost::filesystem::path{filename};
  assert(boost::filesystem::is_regular_file(path));
  return static_cast<size_t>(boost::filesystem::file_size(path));
}

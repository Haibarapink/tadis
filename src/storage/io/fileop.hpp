/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-13 23:46:59
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-30 22:21:05
 * @FilePath: /tadis/src/storage/io/readfile.hpp
 */
#pragma once
#include "common/bytes.hpp"
#include <fstream>
#include <iterator>
#include <string_view>

/**
 *@brief 一次读完全部文件
 */
inline Bytes std_readfile(std::string_view filename)
{
  std::ifstream ifs{filename.data(), std::ios::in | std::ios::binary | std::ios::ate};
  auto filesize = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  Bytes bytes;
  ifs.read(reinterpret_cast<char *>(bytes.data()), filesize);
  ifs.close();
  return bytes;
}

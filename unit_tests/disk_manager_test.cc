/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-02 16:54:47
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-04 22:04:16
 * @FilePath: /tadis/unit_tests/disk_manager_test.cc
 * @Description: Test DiskManager
 */
#include "common/logger.hpp"
#include "storage/io/disk.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <ios>
#include <ostream>

void test()
{
  std::fstream fs{"example.txt", std::ios_base::in | std::ios_base::out | std::ios_base::app};
  if (!fs.is_open()) {
    std::cout << "create file 'example.txt' FAIL" << std::endl;
    return;
  }
  fs.close();
  DiskManager d{"example.txt"};
  std::vector<char> wbuffer(4096, 'h'), rbuffer(4096, 0);

  wbuffer[500] = '6';

  d.write_page(0, wbuffer.data());
  d.write_page(1, wbuffer.data());
  d.read_page(1, rbuffer.data());

  assert(strcmp(wbuffer.data(), rbuffer.data()) == 0);

  // remove("example.txt");
}

int main(int, char *[])
{
  test();
}
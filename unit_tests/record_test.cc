/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-04 20:08:12
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-05 21:38:08
 * @FilePath: /tadis/unit_tests/record_test.cc
 * @Description: record tester
 */
#include "storage/io/buffer_pool.hpp"
#include "storage/io/iodef.hpp"
#include "storage/io/record.hpp"
#include <math.h>
#include <string_view>
#include <cassert>

void record_page_basic_test()
{
  BufferPool bfp{"rec_page_test.db", 3};
  std::vector<PageId> pids(16, INVALID_ID);
  std::vector<Page *> pages(16, nullptr);
  pages[0] = bfp.new_page(pids[0]);
  std::vector<Record> records(16);
  std::vector<Record> records2(16);
  std::string hello_world = "hello world", fuck_world = "fuck world", nihao = "你好";

  // rec1 : hello world
  for (auto ch : hello_world)
    records[0].data().push_back(ch);

  // rec2 : fuck world
  for (auto ch : fuck_world)
    records[1].data().push_back(ch);

  // rec3 : nihao
  for (auto ch : nihao)
    records[2].data().push_back(ch);

  // insert test
  RecordPage rec_page;
  rec_page.init(pages[0]);
  std::vector<RecordId> rids(16);
  assert(rec_page.insert(records[0], rids[0]));
  assert(rids[0].slot_id_ == 0);

  assert(rec_page.insert(records[1], rids[1]));
  assert(rids[1].slot_id_ == 1);

  assert(rec_page.insert(records[2], rids[2]));
  assert(rids[2].slot_id_ == 2);

  bfp.unpin(pids[0], true);

  pages[1] = bfp.new_page(pids[1]);
  pages[2] = bfp.new_page(pids[2]);
  pages[3] = bfp.new_page(pids[3]);
  pages[4] = bfp.new_page(pids[4]);
  pages[5] = bfp.new_page(pids[5]);

  bfp.unpin(pids[1], false);
  bfp.unpin(pids[2], false);
  bfp.unpin(pids[3], false);
  bfp.unpin(pids[4], false);
  bfp.unpin(pids[5], false);

  pages[0] = bfp.fetch(pids[0]);
  RecordPage rec_page2;
  rec_page2.init(pages[0]);

  rec_page2.get(records2[0], rids[0]);
  rec_page2.get(records2[1], rids[1]);
  rec_page2.get(records2[2], rids[2]);

  std::string_view v1{records2[0].data().data(), records2[0].data().size()};
  std::string_view v2{records2[1].data().data(), records2[1].data().size()};
  std::string_view v3{records2[2].data().data(), records2[2].data().size()};

  assert(v1 == hello_world);
  assert(v2 == fuck_world);
  assert(v3 == nihao);

  // flush all dirty page
  bfp.close();

  BufferPool bfp2{"rec_page_test.db"};

  // remove("rec_page_test.db");
}

void record_page_reopen_test()
{
  BufferPool bfp{"rec_page_test2.db", 3};
  RecordPage rec_p;
  auto p = bfp.fetch(1);
  rec_p.init(p);
  RecordId rid;
  rid.page_id_ = 1;
  rid.slot_id_ = 100;
  for (size_t i = 0; i < 200; ++i) {
    Record r;
    rid.slot_id_ = i;
    rec_p.get(r, rid);
    std::string_view hello = std::string_view{r.data().data(), r.data().size()};
    if (hello != "hello world") {
      assert(hello == "hello world");
    } else {
    }
  }
  // remove("rec_page_test.db");
}

void record_page_remain_test()
{
  BufferPool bfp{"rec_page_test2.db", 3};
  std::vector<PageId> pids(16, INVALID_ID);
  std::vector<Page *> pages(16, nullptr);
  pages[0] = bfp.new_page(pids[0]);
  std::vector<Record> records(16);
  std::vector<Record> records2(16);
  std::string hello_world = "hello world", fuck_world = "fuck world", nihao = "你好";

  // rec1 : hello world
  for (auto ch : hello_world)
    records[0].data().push_back(ch);

  // rec2 : fuck world
  for (auto ch : fuck_world)
    records[1].data().push_back(ch);

  // rec3 : nihao
  for (auto ch : nihao)
    records[2].data().push_back(ch);

  // insert test
  RecordPage rec_page;
  rec_page.init(pages[0]);

  size_t stop_i = 0;
  for (auto i = 0; i < 4096; ++i) {
    RecordId rid;
    bool ok = rec_page.insert(records[0], rid);

    if (i == 300) {
      std::cout << "ehll,o";
    }

    if (ok) {
      assert(rid.slot_id_ == stop_i);
      stop_i++;
      // std::cout << "slot_id_ : " << rid.slot_id_ << " stop_i : " << stop_i << std::endl;
    } else {
      break;
    }
  }

  std::cout << "stop_i : " << stop_i << std::endl;

  bfp.unpin(pids[0], true);
  bfp.flush_page(pids[0]);
  bfp.close();

  // remove("rec_page_test.db");
  record_page_reopen_test();
}

int main(int, char *[])
{
  record_page_basic_test();
  record_page_remain_test();
  // std::cout << "====================== record_page_test() pass ===========================" << std::endl;
}
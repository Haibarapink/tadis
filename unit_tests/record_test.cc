/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-02-04 20:08:12
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-06 16:05:54
 * @FilePath: /tadis/unit_tests/record_test.cc
 * @Description: record tester
 */
#include "storage/io/buffer_pool.hpp"
#include "storage/io/iodef.hpp"
#include "storage/io/record.hpp"

#include <cstdlib>
#include <ctime>
#include <math.h>
#include <string>
#include <string_view>
#include <cassert>
#include <vector>

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

  remove("rec_page_test.db");
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
  remove("rec_page_test.db");
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

void record_remove_test(int which)
{
  std::string file = "record_remove.db";
  BufferPool bfp{std::string_view{file.data(), file.size()}, 64};
  std::vector<Page *> pages(1024, nullptr);
  std::vector<PageId> pids(1024, 0);

  // 创建一个page
  for (auto i = 0; i < 1024; ++i) {
    pages[i] = bfp.new_page(pids[i]);
    // fail 则 bfp 有问题
    assert(pages[i]);
    bfp.unpin(pids[i], true);
  }

  auto page = bfp.fetch(which);
  RecordPage recp;
  recp.init(page);

  std::string name = "我是真的服了，好多Bug呀，妈妈咪呀 ----- xp";
  Record record;
  RecordId rid;
  record.append(name.begin(), name.end());
  assert(recp.insert(record, rid));
  assert(recp.contain(rid));
  bfp.unpin(which, true);

  auto page2 = bfp.fetch(which);
  RecordPage recp2;
  recp2.init(page2);
  recp2.remove(rid);

  assert(recp2.contain(rid) == RC::RECORD_IS_DELETED);
  rid.slot_id_ = 1024;
  assert(recp2.contain(rid) == RC::OUT_OF_RANGE);

  bfp.close();
  remove(file.c_str());
}

void record_page_scanner_test()
{
  std::string file = "scanner.db";
  BufferPool bfp{std::string_view{file.data(), file.size()}, 64};
  PageId pid;
  auto page = bfp.new_page(pid);
  auto rp = RecordPage{};
  rp.init(page);

  size_t count = 32;
  std::vector<RecordId> rids(count);
  std::vector<std::string> msgs;

  for (auto i = 0; i < count; ++i) {
    int tm = rand();
    std::string tm_s = std::to_string(tm);
    msgs.emplace_back(tm_s);
    Record rc;
    rc.append(tm_s.begin(), tm_s.end());
    assert(rp.insert(rc, rids[i]));
  }

  PageRecordScanner scanner;
  scanner.init(&rp);
  size_t p = 0;

  while (scanner.has_next()) {
    RecordId runner_rid;
    Record runner_rec;
    scanner.next(runner_rec, runner_rid);
    assert(msgs[p] == runner_rec.to_string());
    assert(rids[p].slot_id_ == runner_rid.slot_id_);
    p++;
  }

  remove(file.c_str());
}

void record_page_scanner_test2()
{
  std::string file = "scanner.db";
  BufferPool bfp{std::string_view{file.data(), file.size()}, 64};
  PageId pid;
  auto page = bfp.new_page(pid);
  auto rp = RecordPage{};
  rp.init(page);
  PageRecordScanner scanner;
  scanner.init(&rp);
  size_t p = 0;

  while (scanner.has_next()) {
    RecordId runner_rid;
    Record runner_rec;
    scanner.next(runner_rec, runner_rid);
    p++;
  }

  assert(p == 0);

  remove(file.c_str());
}

std::string rand_data()
{
  auto data = rand();
  return std::to_string(data);
}

void table_scanner_test()
{
  std::string file = "scanner.db";
  BufferPool bfp{std::string_view{file.data(), file.size()}, 16};
  size_t page_size = 32;
  size_t one_page_record_size = 0;

  std::vector<std::string> datas;
  std::vector<PageId> pids;
  std::vector<RecordId> rids;

  for (size_t i = 0; i < page_size; ++i) {
    PageId pid;
    auto new_page = bfp.new_page(pid);
    pids.push_back(pid);

    RecordPage rcp{new_page};
    assert(new_page->pid() == i + 1);
    // 一直插入 直到插不进去
    while (true) {
      auto str = rand_data();
      Record rec{str.begin(), str.end()};
      RecordId rid;
      auto ok = rcp.insert(rec, rid);
      if (!ok)
        break;
      rids.push_back(rid);
      datas.emplace_back(std::move(str));
    }

    bfp.unpin(pid, true);
  }

  RecordScanner scanner;
  assert(scanner.init(&bfp));
  Record record;
  RecordId rid;

  auto rid_iter = rids.begin();
  auto datas_iter = datas.begin();
  size_t p = 0;
  while (scanner.has_next()) {
    RC rc = scanner.next(record, rid);
    assert(rc == RC::SUCCESS);

    if (rids[p] != rid) {
      assert(false);
    }

    if (record.to_string() != datas[p]) {
      assert(false);
    }

    rid_iter++;
    datas_iter++;

    record.reset();
    rid.page_id_ = INVALID_ID;
    rid.slot_id_ = INVALID_ID;
    p++;
  }

  bfp.flush_all_page();

  remove(file.data());
}

int main(int, char *[])
{
  // record_page_basic_test();
  // record_page_remain_test();
  // for (auto i = 0; i < 1024; ++i)
  //   record_remove_test(104);
  // record_page_scanner_test();
  // record_page_scanner_test2();
  table_scanner_test();
  // std::cout << "====================== record_page_test() pass ===========================" << std::endl;
}
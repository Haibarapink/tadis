#include "storage/io/buffer_pool.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <math.h>

class BFPTester {
public:
  auto &&get_dir()
  {
    return bp_->dir_;
  }

  auto &&get_replacer()
  {
    return bp_->replacer_;
  }

  auto &&get_free_list()
  {
    return bp_->free_list_;
  }

  auto &&hd()
  {
    return bp_->head_;
  }

  auto bfp_map()
  {
    return bp_->head_.bitmap();
  }

  BufferPool *bp_;
};

void basic_test()
{
  BufferPool bp{"test1.db", 3};
  BFPTester t;
  t.bp_ = &bp;

  assert(t.get_free_list().size() == 2);
  assert(t.get_dir().size() == 1);

  PageId pid1{INVALID_ID}, pid2{INVALID_ID}, pid3{INVALID_ID}, pid4{INVALID_ID}, pid5{INVALID_ID}, pid6{INVALID_ID},
      pid7{INVALID_ID};
  auto page1 = bp.new_page(pid1);
  auto page2 = bp.new_page(pid2);
  auto page3 = bp.new_page(pid3);

  // should fail
  auto page4 = bp.new_page(pid4);

  assert(pid1 == 1 && pid2 == 2 && pid3 == 3 && pid4 == INVALID_ID);
  assert(t.get_dir().size() == 3);
  assert(page4 == nullptr);

  assert(t.hd().bitmap().get(1) && t.hd().bitmap().get(0) && t.hd().bitmap().get(2));

  std::string hello_world = "hello world";
  memcpy(page1->data(), hello_world.data(), hello_world.size());

  // unpin 1
  bp.unpin(pid1, true);

  page4 = bp.new_page(pid4);
  assert(t.hd().bitmap().get(3));
  std::string fuck_world = "fuck world";
  memcpy(page4->data(), fuck_world.data(), fuck_world.size());

  // unpin 4
  bp.unpin(pid4, true);

  // page1 应该给 pid4用了
  assert(page1->pid() == pid4);

  bp.unpin(pid2, true);
  bp.unpin(pid3, true);

  auto page5 = bp.new_page(pid5);
  auto page6 = bp.new_page(pid6);
  auto page7 = bp.new_page(pid7);

  assert(page5 && page6 && page7);

  sprintf(page7->data(), "I am page7");

  bp.unpin(pid5, false);
  bp.unpin(pid6, false);
  bp.unpin(pid7, false);

  page1 = bp.fetch(pid1);
  page4 = bp.fetch(pid4);

  assert(page1 && page4);
  std::vector<char> p1_data(hello_world.size(), 0);
  std::vector<char> p4_data(fuck_world.size(), 0);

  memcpy(p1_data.data(), page1->data(), hello_world.size());
  memcpy(p4_data.data(), page4->data(), fuck_world.size());

  std::string_view p1_view{page1->data(), hello_world.size()};
  std::string_view p4_view{page4->data(), fuck_world.size()};

  bp.unpin(pid1, false);
  bp.unpin(pid4, false);

  page7 = bp.fetch(pid7);

  assert(p1_view == "hello world");
  assert(p4_view == "fuck world");

  bp.close();
  BufferPool bp2{"test1.db"};

  remove("test1.db");
}

void reopen_test()
{
  std::string file = "reopen_test.db";
  PageId pid;
  BufferPool bfp1{std::string_view{file.data(), file.size()}};
  bfp1.new_page(pid);
  bfp1.unpin(pid, true);
  bfp1.close();

  BufferPool bfp2{std::string_view{file.data(), file.size()}};
  BFPTester t{&bfp2};
  auto bitmap = t.bfp_map();
  std::cout << t.hd().page_num_ << std::endl;
  std::cout << "bitmap" << bitmap.to_string();
}

void reopen_test2()
{
  std::string_view file = "rp2.db";
  BufferPool bfp{file, 16};
  size_t count = 256;

  for (auto i = 0; i < count; ++i) {
    PageId id;
    auto p = bfp.new_page(id);
    bfp.unpin(id, true);
  }
  bfp.flush_all_page();
  bfp.close();

  BufferPool bfp2{std::string_view{file.data(), file.size()}};
  BFPTester t{&bfp2};
  assert(t.hd().phy_num_ == count + 1);
}

void many_test(size_t time)
{
  std::string_view file = "many.db";
  BufferPool bfp{file, 16};
  size_t count = 256;

  for (auto i = 0; i < count; ++i) {
    PageId id;
    auto p = bfp.new_page(id);
    if (id != i + 1) {
      remove(file.data());
      assert(false);
    }
    bfp.unpin(id, true);
  }
  bfp.flush_all_page();
  bfp.close();
}

int main(int, char *[])
{
  // basic_test();
  // reopen_test();
  // reopen_test2();
  many_test(0);
}
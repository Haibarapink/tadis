#include "storage/io/buffer_pool.hpp"
#include <cassert>
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

  BufferPool *bp_;
};

void basic_test()
{
  BufferPool bp{"test.db", 3};
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

  bp.unpin(pid2, false);
  bp.unpin(pid3, false);

  auto page5 = bp.new_page(pid5);
  auto page6 = bp.new_page(pid6);
  auto page7 = bp.new_page(pid7);

  assert(page5 && page6 && page7);

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

  // assert(p1_view == "hello world");
  // assert(p4_view == "fuck world");
  // remove("test.db");
}

int main(int, char *[])
{
  basic_test();
}
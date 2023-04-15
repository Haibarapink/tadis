#include "storage/bufferpool/buffer_pool.hpp"
#include "pure.hpp"
#include "storage/table_heap/table_heap.hpp"
#include "storage/bufferpool/record.hpp"
PURE_TEST_INIT();

class BufferPoolTest {
public:
  void buffer_pool_clean_test() {
    TableHeap heap{"test.db"};

    std::vector<char> buffer(100);
    for (auto i = 0; i < 10000; ++i) {
      Record r;
      std::string s = std::to_string(i) + "hello";
      r.bytes()  = std::vector<char>(s.begin(), s.end());
      pure_assert(heap.insert(r)) << s;
    }

    auto scanner = heap.scanner();
    int i = 0;
    while (scanner->has_next()) {
      Record r;
      RID rid;
      auto rc = scanner->next(r, rid);
      std::string s = std::to_string(i) + "hello";
      std::string_view v = std::string_view{r.bytes().data(), r.bytes().size()};
      pure_assert(s == v) << "s : " << s << " " << r.bytes().size();
      i++;
    }

    heap.close();

    auto bfp = heap.buffer_pool();

    for (auto &p : bfp->pages_ ) {
      pure_assert(p->pin_count_ == 0) << p->pin_count_ << " " << p->pid_;
    }


    remove("test.db");
  }
};

void buffer_pool_clean_test() {
    BufferPoolTest t;
    t.buffer_pool_clean_test();
}

void signal_page_op_test() {
  BufferPoolTest t;
}

int main(int, char **)
{
  PURE_TEST_PREPARE();
  PURE_TEST_CASE(buffer_pool_clean_test);
  PURE_TEST_RUN();
}
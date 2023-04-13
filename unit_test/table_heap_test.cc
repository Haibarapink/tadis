#include "storage/bufferpool/buffer_pool.hpp"
#include "pure.hpp"
#include "storage/table_heap/table_heap.hpp"
#include "storage/bufferpool/record.hpp"
PURE_TEST_INIT();

class BufferPoolTest {
public:
  void buffer_pool_clean_test() {
    BufferPool bfp{"test.db", 12};



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
  PURE_TEST_CASE(signal_page_op_test);
  PURE_TEST_RUN();
}
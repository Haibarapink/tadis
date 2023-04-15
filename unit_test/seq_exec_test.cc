#include "execution/seqscan_executer.hpp"
#include "pure.hpp"
#include "storage/table_heap/table_heap.hpp"

PURE_TEST_INIT();

void basic_test()
{
  TableHeap tp{"test.db"};
  std::vector<std::vector<char>> bytes;

  for (auto i = 0; i < 300000; ++i) {
    std::string s = "hello" + std::to_string(i);
    auto b = std::vector<char>{s.begin(), s.end()};
    Record r;
    r.bytes() = b;
    tp.insert(r);
    bytes.emplace_back(std::move(b));
  }

  Tuple t{{}};
  Schema schema{{}};
  size_t p = 0;
  SeqScanExectuer exec{tp.scanner(), &schema};
  while (rc_success(exec.next(&t))) {
    pure_assert(t.data_ == bytes[p]);
    p++;
  }
  tp.close();
  remove("test.db");
}

int main() {
  PURE_TEST_PREPARE();

  PURE_TEST_CASE(basic_test);

  PURE_TEST_RUN();
}
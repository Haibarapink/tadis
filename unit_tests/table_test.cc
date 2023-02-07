#include "storage/table.hpp"
#include "storage/tuple.hpp"
#include <cassert>
#include <vector>

void basic_test()
{
  TableMeta meta;
  std::vector<TupleCellMeta> metas;
  metas.push_back(TupleCellMeta::init("name", TupleCellType::VARCHAR));
  metas.push_back(TupleCellMeta::init("age", TupleCellType::INTEGER));

  meta.init("student", metas);

  Table table;
  table.init("", meta);

  //   table.insert(const std::vector<Value> &value)
}

int main(int argc, char *[])
{
  basic_test();
}
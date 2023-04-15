#include "catalog/column.hpp"
#include "catalog/tuple.hpp"
#include "pure.hpp"
#include "sql/parser/ast.hpp"
#include <vector>

PURE_TEST_INIT();


void vpv_test()
{
  std::vector<Column> cols;
  cols.push_back(Column(ColumnType::INT, "id", sizeof(int)));
  cols.push_back(Column(ColumnType::FLOAT, "score", sizeof(float)));
  cols.push_back(Column(ColumnType::CHAR, "name", 10));

  auto values = init_values(1, 2.3f, "helloworld");
  pure_assert(values[0].type_ == AttrType::INTS);
  pure_assert(values[1].type_ == AttrType::FLOATS);
  pure_assert(values[2].type_ == AttrType::STRING);
  auto str = values[2].get<std::string>();

  Schema schema{cols};
  auto tuple = Tuple::create_tuple(values, &schema);

  auto v = tuple.value_at(&schema, 0);
  pure_assert(v.get<int>() == 1) << v.get<int>();

  v = tuple.value_at(&schema, 1);
  pure_assert(v.get<float>() == 2.3f) << v.get<float>();

  v = tuple.value_at(&schema, 2);

  std::string hello = std::string("helloworld");  // ub !!!!!
  pure_assert(v.get<std::string>() == hello);
}

// varchar
void varchar_test()
{
  std::vector<Column> cols;
  cols.push_back(Column(ColumnType::INT, "id", sizeof(int)));
  cols.push_back(Column(ColumnType::FLOAT, "score", sizeof(float)));
  cols.push_back(Column(ColumnType::VARCHAR, "name", 0));

  auto values = init_values(1, 2.3f, "helloworld and duck");
  pure_assert(values[0].type_ == AttrType::INTS);
  pure_assert(values[1].type_ == AttrType::FLOATS);
  pure_assert(values[2].type_ == AttrType::STRING);
  auto str = values[2].get<std::string>();

  Schema schema{cols};
  auto tuple = Tuple::create_tuple(values, &schema);

  auto v = tuple.value_at(&schema, 0);
  pure_assert(v.get<int>() == 1) << v.get<int>();

  v = tuple.value_at(&schema, 1);
  pure_assert(v.get<float>() == 2.3f) << v.get<float>();

  v = tuple.value_at(&schema, 2);

  std::string hello = std::string("helloworld and duck");  // ub !!!!!
  pure_assert(v.get<std::string>() == hello);
}

void varchar2_test()
{
  std::vector<Column> cols;
  cols.push_back(Column(ColumnType::INT, "id", sizeof(int)));
  cols.push_back(Column(ColumnType::FLOAT, "score", sizeof(float)));
  cols.push_back(Column(ColumnType::INT, "id2", sizeof(int)));
  cols.push_back(Column(ColumnType::VARCHAR, "name", 0));

  auto values = init_values(1, 2.3f, 2,"helloworld and duck");
  pure_assert(values[0].type_ == AttrType::INTS);
  pure_assert(values[1].type_ == AttrType::FLOATS);
  pure_assert(values[2].type_ == AttrType::INTS);
  auto str = values[3].get<std::string>();

  Schema schema{cols};
  auto tuple = Tuple::create_tuple(values, &schema);

  auto v = tuple.value_at(&schema, 0);
  pure_assert(v.get<int>() == 1) << v.get<int>();

  v = tuple.value_at(&schema, 1);
  pure_assert(v.get<float>() == 2.3f) << v.get<float>();

  v = tuple.value_at(&schema, 3);

  std::string hello = std::string("helloworld and duck");
  pure_assert(v.get<std::string>() == hello);
}

int main(int, char **)
{
  PURE_TEST_PREPARE();
  PURE_TEST_CASE(vpv_test);
  PURE_TEST_CASE(varchar_test);
  PURE_TEST_CASE(varchar2_test);
  PURE_TEST_RUN();
}
#include "common/json.hpp"
#include "pure.hpp"
#include "json/json_parser.hpp"
#include "json/json_value.hpp"
#include "catalog/column.hpp"
#include "catalog/schema.hpp"

PURE_TEST_INIT();
class parser_test {
public:
  void basic_test()
  {
    std::string s = "{\"hello\" : \"world\"}";
    pson::Parser p{s};
    pson::Value v;
    v.reset_as(pson::JSON_STRING);
    pure_assert(p.Parse(v));
    auto v2 = v["hello"];
    auto str = v2.as<pson::String>();
    pure_assert(str == "world");
  }
};

void vpv_test()
{
  parser_test t;
  t.basic_test();
}

void column_test()
{
  Column c(ColumnType::FLOAT, "hello", 10);
  auto v = c.to_json();
  Column c2(ColumnType::FLOAT, "FU", 66);
  c2.from_json(v);
  pure_assert(c.type() == c2.type());
  pure_assert(c.name() == c2.name());
  pure_assert(c.size() == c2.size());
}

void schema_test() {
  std::vector<Column> cols;
  cols.emplace_back(ColumnType::FLOAT, "hello", 10);
  cols.emplace_back(ColumnType::VARCHAR, "world", 11);
  cols.emplace_back(ColumnType::CHAR, "day", 12);
  Schema schema{cols};
  auto v = schema.to_json();
  Schema schema1{{}};
  schema1.from_json(v);

  assert(schema1 == schema);
}

void write_file_test() {
    pson::Value v;
    v.reset_as(pson::JSON_OBJECT);
    v.insert("hello", "world");
    JsonFileRWer rwer;
    rwer.write("test.json", v);
    pson::Value v2;
    rwer.read("test.json", v2);

    assert(v2.is_object());
}

int main(int, char **)
{
  PURE_TEST_PREPARE();
  PURE_TEST_CASE(vpv_test);
  PURE_TEST_CASE(column_test);
  PURE_TEST_CASE(schema_test);
  PURE_TEST_CASE(write_file_test);
  PURE_TEST_RUN();
}
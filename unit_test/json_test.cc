#include "common/json.hpp"
#include "pure.hpp"
#include "json/json_parser.hpp"
#include "json/json_value.hpp"

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

int main(int, char **)
{
  PURE_TEST_PREPARE();
  PURE_TEST_CASE(vpv_test);
  PURE_TEST_RUN();
}
#include "binder/binder.hpp"
#include "sql/parser/parser.hpp"
#include "pure.hpp"

PURE_TEST_INIT();

SqlStmt make_stmt(std::string str) {
  Parser<std::string> parser{str};
  auto ok = parser.do_parse();
  pure_assert(ok == RC::SUCCESS);
  return SqlStmt{std::move(parser.query())};
}

class BinderTest {
public:
  void basic_test()
  {
    Catalog ct;
    std::vector<Column> cols;
    cols.emplace_back(ColumnType::INT, "id", 0);
    cols.emplace_back(ColumnType::INT, "age", 0);
    cols.emplace_back(ColumnType::VARCHAR, "name", 0);
    ct.create_table("student", cols);
    ExecuterContext ctx{&ct};

    Binder b{&ctx};

    auto stmt = make_stmt("SELECT student.id, name from student where id > age;");
    auto exec = b.bind(&stmt);
    pure_assert(exec);
    Tuple t{{}};
    RC rc = exec->next(&t);
    pure_assert(!rc_success(rc));
  }

};

void basic_test() {
  BinderTest().basic_test();
}

int main(int, char**) {
  PURE_TEST_PREPARE();
  PURE_TEST_CASE(basic_test);
  PURE_TEST_RUN();
}
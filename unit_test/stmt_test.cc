#include "sql/parser/ast.hpp"

#include "pure.hpp"

PURE_TEST_INIT();

void vpv_test()
{
  QueryStmt stmt = SelectStmt{};
  SqlStmt sql_stmt{stmt};
  pure_assert(!sql_stmt.where_is_vaild());
}

int main(int argc, char **)
{
  PURE_TEST_PREPARE();
  PURE_TEST_CASE(vpv_test);
  PURE_TEST_RUN();
}
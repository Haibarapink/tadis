#include "binder/binder.hpp"
#include "sql/parser/parser.hpp"
#include "pure.hpp"

PURE_TEST_INIT();

SqlStmt make_stmt(std::string str)
{
  Parser<std::string> parser{str};
  auto ok = parser.do_parse();
  pure_assert(ok == RC::SUCCESS) << rc2str(ok);
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
    auto exec = std::unique_ptr<Executer>(b.bind(&stmt));
    pure_assert(exec.get());
    Tuple t{{}};
    RC rc = exec->next(&t);
    pure_assert(!rc_success(rc));

    remove("student.db");
  }

  void create_table_test()
  {
    Catalog ct;
    ExecuterContext ctx{&ct};

    auto stmt = make_stmt("Create table student (id int, age int, name varchar(10));");
    Binder b{&ctx};
    auto exec = std::unique_ptr<Executer>(b.bind(&stmt));
    pure_assert(exec.get());
    RC rc = exec->next(nullptr);
    pure_assert(rc == RC::OUT_OF_RANGE);
    pure_assert(ct.has_table("student"));
    remove("student.db");
  }

  void insert_and_select_test()
  {
    Catalog ct;
    ExecuterContext ctx{&ct};

    auto stmt = make_stmt("Create table student (id int, age int, name varchar(10));");
    Binder b{&ctx};
    auto exec = std::unique_ptr<Executer>(b.bind(&stmt));
    pure_assert(exec.get());
    RC rc = exec->next(nullptr);
    pure_assert(rc == RC::OUT_OF_RANGE);
    pure_assert(ct.has_table("student"));

    stmt = make_stmt("insert into student values (0, 0, 'pink0');");
    exec.reset(b.bind(&stmt));
    pure_assert(exec.get());
    pure_assert(exec->next(nullptr) == RC::OUT_OF_RANGE);

    stmt = make_stmt("insert into student values (1, 1, 'pink1');");
    exec.reset(b.bind(&stmt));
    pure_assert(exec.get());
    pure_assert(exec->next(nullptr) == RC::OUT_OF_RANGE);

    stmt = make_stmt("insert into student values (2, 2, 'pink1');");
    exec.reset(b.bind(&stmt));
    pure_assert(exec.get());
    pure_assert(exec->next(nullptr) == RC::OUT_OF_RANGE);

    stmt = make_stmt("select * from student where age > 1;");
    exec.reset(b.bind(&stmt));
    pure_assert(exec.get());
    Tuple t{{}};
    rc = exec->next(&t);
    pure_assert(rc == RC::SUCCESS);
    auto table = ct.get_table("student");
    std::cout << t.to_string(ctx.result_.output_schema_.get()) << std::endl;

    remove("student.db");
  }
};

void basic_test()
{
  BinderTest().basic_test();
}

void create_table_test()
{
  BinderTest().create_table_test();
}

void insert_select_test()
{
  BinderTest().insert_and_select_test();
}

int main(int, char **)
{
  PURE_TEST_PREPARE();
  PURE_TEST_CASE(basic_test);
  PURE_TEST_CASE(create_table_test);
  PURE_TEST_CASE(insert_select_test);
  PURE_TEST_RUN();
}
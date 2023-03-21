#include "catalog/catalog.hpp"
#include "sql/parser/parer_stage.hpp"
#include "execution/exec_stage.hpp"
#include <cassert>

void exec_query(std::function<void(int where)> callback, std::string query)
{
  auto s = std::unique_ptr<Stage>(new ParsingStage{});
  ParsingStage *p = (ParsingStage *)s.get();
  p->init(query);
  s->do_request();
  callback(__LINE__);
}

void create_table()
{
  auto s = std::unique_ptr<Stage>(new ParsingStage{});
  ParsingStage *p = (ParsingStage *)s.get();

  std::string query = "Create table t (name varchar(10), age int);";
  p->init(query);
  s->do_request();

  auto tm = Catalog::catalog().table_manager();
  assert(tm->contain(std::string("t")));

  query = "insert into t values ('pink', 100);";
}

void insert_test()
{
  std::string query1 = "Create table t (name varchar(10), age int);";
  std::string query2 = "insert into t values ('pink', 100);";
  exec_query(
      [&](int where) {
        auto tm = Catalog::catalog().table_manager();
        if (!tm->contain(std::string("t"))) {
          std::cout << "assert at" << where << std::endl;
          assert(false);
        }
      },
      query1);

  exec_query([&](int where) {}, query2);
}

int main(int argc, char *argv[])
{
  Catalog::catalog().init("./.tadis");
  insert_test();
  Catalog::catalog().table_manager()->close();
}
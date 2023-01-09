/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-06 16:50:35
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-09 13:34:18
 * @FilePath: /tadis/src/tests/parse_test.cc
 * @Description: test
 */
#include <boost/config/workaround.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/test/detail/global_typedef.hpp>
#include <sql/parser/parser.hpp>
#include <string>

void basic_test()
{
  std::string s = "select * from school;";
  Parser<std::string> p(s);
  BOOST_TEST(p.parse() == RC::SUCCESS);
  BOOST_TEST(std::get<Select>(p.query()).selist_.front().attribute_ == "*");
}

void selist_test()
{
  std::string s = "select age, name, school.name from school;";
  Parser<std::string> p(s);
  BOOST_TEST(p.parse() == RC::SUCCESS);
  BOOST_TEST(std::get<Select>(p.query()).selist_[0].attribute_ == "age");
  BOOST_TEST(std::get<Select>(p.query()).selist_[1].attribute_ == "name");
  BOOST_TEST(std::get<Select>(p.query()).selist_[2].attribute_ == "name");
  BOOST_TEST(std::get<Select>(p.query()).selist_[2].table_ == "school");
}

void from_test()
{
  std::string s = "select * from school, student, teacher;";
  std::vector<std::string> tables = {"school", "student", "teacher"};
  Parser<std::string> p(s);
  BOOST_TEST(p.parse() == RC::SUCCESS);
  auto select = std::get<Select>(p.query());
  BOOST_TEST(select.selist_[0].attribute_ == "*");
  for (auto i = 0; i < tables.size(); ++i) {
    BOOST_TEST_EQ(select.from_list_[i], tables[i]);
  }
}

void where_test()
{
  std::string s = "select * from school, student, teacher where student.name=\'XuPing\', school.name=\'AAU\', "
                  "student.id=1111, 2=3;";
  std::vector<std::string> tables = {"school", "student", "teacher"};
  Parser<std::string> p(s);
  BOOST_TEST(p.parse() == RC::SUCCESS);
  auto select = std::get<Select>(p.query());
  BOOST_TEST(select.selist_[0].attribute_ == "*");
  for (auto i = 0; i < tables.size(); ++i) {
    BOOST_TEST_EQ(select.from_list_[i], tables[i]);
  }
  BOOST_TEST(select.cond_list_[0].left_.get<RelAttr>().attribute_ == "name");
  BOOST_TEST(select.cond_list_[0].left_.get<RelAttr>().table_ == "student");
  BOOST_TEST(select.cond_list_[0].right_.get<std::string>() == "XuPing");
  BOOST_TEST(select.cond_list_[0].op_ == CondOp::EQ);

  BOOST_TEST(select.cond_list_[3].left_.get<long>() == 2);
  BOOST_TEST(select.cond_list_[3].right_.get<long>() == 3);
  BOOST_TEST(select.cond_list_[3].op_ == CondOp::EQ);
}

void where_test2()
{
  std::string s = "select * from school, student, teacher where student.name=1, school.name=2, "
                  "student.id=1111;";
  std::vector<std::string> tables = {"school", "student", "teacher"};
  Parser<std::string> p(s);
  BOOST_TEST(p.parse() == RC::SUCCESS);
  auto select = std::get<Select>(p.query());
  BOOST_TEST(select.selist_[0].attribute_ == "*");
  for (auto i = 0; i < tables.size(); ++i) {
    BOOST_TEST_EQ(select.from_list_[i], tables[i]);
  }

  BOOST_TEST(select.cond_list_[0].left_.get<RelAttr>().attribute_ == "name");
  BOOST_TEST(select.cond_list_[0].right_.get<long>() == 1);
  BOOST_TEST(select.cond_list_[0].op_ == CondOp::EQ);

  BOOST_TEST(select.cond_list_[2].left_.get<RelAttr>().attribute_ == "id");
  BOOST_TEST(select.cond_list_[2].right_.get<long>() == 1111);
  BOOST_TEST(select.cond_list_[2].op_ == CondOp::EQ);
}

void delete_test1()
{
  std::string s = "DELETE FROM t WHERE t.name='allo' , t.age=2;";
  Parser<std::string> p{s};
  BOOST_TEST(p.parse() == RC::SUCCESS);
  auto d = std::get<Delete>(p.query());
  BOOST_TEST(d.tables_[0] == "t");
  BOOST_TEST(d.conds_[0].left_.get<RelAttr>().attribute_ == "name");
  BOOST_TEST(d.conds_[0].right_.get<std::string>() == "allo");

  BOOST_TEST(d.conds_[1].left_.get<RelAttr>().attribute_ == "age");
  BOOST_TEST(d.conds_[1].right_.get<long>() == 2);
}

void delete_test_fail1()
{
  std::string s = "DELETE * FROM t WHERE t.name='allo' , t.age=2;";
  Parser<std::string> p{s};
  BOOST_TEST(p.parse() == RC::SYNTAX_ERROR);
}

void insert_test1()
{
  std::string s = "INSERT INTO t VALUES (1,0.01, 'hello');";
  Parser<std::string> parser{s};
  BOOST_TEST(parser.parse() == RC::SUCCESS);
  auto insert = std::get<Insert>(parser.query());
  BOOST_TEST(insert.table_name_ == "t");
  BOOST_TEST(insert.values_[0].get<long>() == 1);
  //  BOOST_TEST(insert.values_[1].get<float>() == 0.01);
  BOOST_TEST(insert.values_[2].get<std::string>() == "hello");
}

void insert_test2()
{
  std::string s = "INSERT INTO t (name, age, num) VALUES ('hello', 2,3);";
}

void create_test1()
{
  std::string s = "CREATE TABLE t (name varchar(100), age int, school char(100));";
  Parser<std::string> parser{s};
  BOOST_TEST(parser.parse() == RC::SUCCESS);
  auto c = std::get<CreateTable>(parser.query());
  BOOST_TEST(c.table_name_ == "t");
  BOOST_TEST(c.col_attrs_[0].name_ == "name");
  BOOST_TEST(c.col_attrs_[0].type_ == "VARCHAR");
  BOOST_TEST(c.col_attrs_[0].size_ == 100);
  BOOST_TEST(c.col_attrs_[1].name_ == "age");
  BOOST_TEST(c.col_attrs_[1].type_ == "INT");
  BOOST_TEST(c.col_attrs_[1].size_ == 0);
  BOOST_TEST(c.col_attrs_[2].name_ == "school");
  BOOST_TEST(c.col_attrs_[2].type_ == "CHAR");
  BOOST_TEST(c.col_attrs_[2].size_ == 100);
}

void create_test2()
{
  std::string s = "CREATE TABLE t (age float, school char(100));";
  Parser<std::string> parser{s};
  BOOST_TEST(parser.parse() == RC::SUCCESS);
  auto c = std::get<CreateTable>(parser.query());
  BOOST_TEST(c.table_name_ == "t");
  BOOST_TEST(c.col_attrs_[0].name_ == "age");
  BOOST_TEST(c.col_attrs_[0].type_ == "FLOAT");
}

int main(int argc, char *[])
{
  basic_test();
  selist_test();
  from_test();
  where_test();
  where_test2();

  delete_test1();
  delete_test_fail1();

  insert_test1();

  create_test1();
  create_test2();
  return boost::report_errors();
}
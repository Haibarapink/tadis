/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-06 16:50:35
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-08 12:46:35
 * @FilePath: /tadis/src/tests/parse_test.cc
 * @Description: test
 */
#include "sql/parser/parser_define.hpp"
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
  std::string s = "select * from school, student, teacher where student.name=\"XuPing\", school.name=\"AAU\", "
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

int main(int argc, char *[])
{
  basic_test();
  selist_test();
  from_test();
  where_test();
  where_test2();
  return boost::report_errors();
}
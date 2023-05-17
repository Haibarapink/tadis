#include "bin/tabulate.h"

#include <iostream>
#include <memory>
#include <string>
#include "bin/cmd.hpp"
#include "binder/binder.hpp"
#include "catalog/catalog.hpp"
#include "common/rc.hpp"
#include "execution/execution.hpp"
#include "execution/result.hpp"
#include "sql/parser/ast.hpp"
#include "sql/parser/parser.hpp"
#include "execution/executer_context.hpp"

Command cmd;
std::unique_ptr<Catalog> catalog = nullptr;
bool quit = false;

void show_tables() {
  tabulate::Table constant;
  auto rows = tabulate::Table::Row_t{};
  auto tables = catalog->tables_name();
  for (auto& name : tables) {
    rows.emplace_back(std::move(name));
  }
  constant.add_row(rows);
  std::cout << constant << std::endl;
}

void report(const QueryResult &res)
{
  if (res.rc_ != RC::SUCCESS) {
    std::cout << "FAIL " << rc2str(res.rc_) << " " << res.msg_ << std::endl;
  }
}

void print_rows(ExecuterContext &ctx)
{
  if (ctx.result_.type_ != QueryType::SELECT) {
    return;
  }
  std::cout << "count: " << ctx.result_.tuples_.size() << std::endl;
  tabulate::Table constant;

  auto schema = ctx.result_.output_schema_.get();
  // add projection columns' name
  tabulate::Table::Row_t schema_name;
  for (auto &i : schema->columns_) {
    schema_name.push_back(i.name());
  }
  constant.add_row(schema_name);

  // add projection columns' values
  tabulate::Table::Row_t val_rows;
  auto out_schema = ctx.result_.output_schema_.get();
  size_t val_num = out_schema->columns_.size();
  for (auto& i : ctx.result_.tuples_) {
    for (auto j = 0; j < val_num; ++j) {
      auto val = i.value_at(out_schema, j);
      val_rows.push_back(val.to_string());
    }
    constant.add_row(val_rows);
    val_rows.clear();
  }

  std::cout << constant << std::endl;
}

std::unique_ptr<ExecuterContext> execute_sql(std::string sql)
{
  std::unique_ptr<ExecuterContext> ctx = std::make_unique<ExecuterContext>(catalog.get());
  Binder binder{ctx.get()};
  Parser<std::string> parser{sql};

  RC rc = parser.do_parse();

  if (!rc_success(rc)) {
    ctx->result_.rc_ = rc;
    ctx->result_.msg_ = std::string{rc2str(rc)} + " " + sql;
    return ctx;
  }

  SqlStmt stmt = SqlStmt{parser.query()};
  auto exec = std::unique_ptr<Executer>(binder.bind(&stmt));
  Tuple t{{}};
  rc = RC::SUCCESS;

  while (rc_success(rc)) {
    rc = exec->next(&t);
    if (rc == RC::SUCCESS) {
      ctx->result_.tuples_.push_back(t);
    }
  }

  if (rc != RC::OUT_OF_RANGE) {
    ctx->result_.rc_ = rc;
    ctx->result_.msg_ = std::string{rc2str(rc)} + " " + sql;
    return ctx;
  }

  ctx->result_.rc_ = RC::SUCCESS;
  return ctx;
}

void exec_from_file(const std::string &filename)
{
  std::fstream fs;
  fs.open(filename);
  if (!fs.is_open()) {
    return;
  }
  std::string line;
  while (std::getline(fs, line)) {
    auto exec_ctx = execute_sql(line);
    if (exec_ctx->result_.rc_ != RC::SUCCESS) {
      report(exec_ctx->result_);
    } else {
      print_rows(*exec_ctx);
    }
  }
}

void init(const std::string &path)
{
  catalog = std::unique_ptr<Catalog>{new Catalog{path}};
  cmd.add(".quit", [&]{
    quit = true;
  });
  cmd.add(".show tables", show_tables);
}

int main(int argc, char **argv)
{
  init(".");

  if (argc > 1) {
    return 0;
  }

  std::string in;
  while (!quit) {
    std::cout << "TadisSQL>";
    std::getline(std::cin, in);
    if (!in.empty() && in.front() == '.') {
      cmd.run(in);
    } else {
        auto exec_ctx = execute_sql(in);
        if (exec_ctx->result_.rc_ != RC::SUCCESS) {
            report(exec_ctx->result_);
        } else {
            std::cout << "Success" << std::endl;
            print_rows(*exec_ctx);
        }
    }
  }
}
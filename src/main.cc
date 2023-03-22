#include "catalog/catalog.hpp"
#include "common/rc.hpp"
#include "result/result_stage.hpp"
#include "sql/parser/parer_stage.hpp"
#include "execution/exec_stage.hpp"
#include <cassert>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

bool quit = false;

void result_cb(std::vector<Tuple> &ts, RC rc)
{
  for (auto &t : ts) {
    std::cout << t.to_string() << std::endl;
  }
}

void handle_error(const std::string &query, RC rc)
{
  std::cout << "\033[31m"
            << "Error: \n"
            << "        query: '" << query << "' " << rc2str(rc) << "\033[0m" << std::endl;
}

void exec_query(std::string query)
{
  std::string q = query;
  auto s = std::unique_ptr<Stage>(new ParsingStage{});
  ParsingStage *p = (ParsingStage *)s.get();
  p->init(std::move(query));
  RC rc = s->do_request();
  if (!rc_success(rc)) {
    handle_error(q, rc);
  }
}

void handle_cmd(const std::string &cmd)
{
  if (cmd == ".quit") {
    quit = true;
  } else {
    std::string_view exec = ".exec";
    std::string_view view = cmd.size() > exec.size() ? std::string_view{cmd.data(), exec.size()} : "";
    if (view == exec) {}
  }
}

int main(int argc, char *argv[])
{
  if (!std::filesystem::exists(".tadis")) {
    std::filesystem::create_directory(".tadis");
  }
  Catalog::catalog().init(".tadis");
  ResultStage::defaultCb = result_cb;

  std::string q;
  while (!quit) {
    std::cout << "\033[32m"
              << "<Tadis 0.0.2>$ "
              << "\033[0m";
    std::getline(std::cin, q);
    if (q.size() > 0 && q[0] == '.')
      handle_cmd(q);
    else
      exec_query(std::move(q));
  }

  Catalog::catalog().table_manager()->close();
}
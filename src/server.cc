#include "catalog/catalog.hpp"
#include "common/rc.hpp"
#include "result/result_stage.hpp"
#include "sql/parser/parer_stage.hpp"
#include "execution/exec_stage.hpp"

#include <sstream>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>



#include <cassert>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

bool quit = false;

std::string result2str(std::vector<Tuple> &ts, RC rc)
{
  std::string result = "";
  if (ts.empty()) {
    result += "Empty set\n";
    return result;
  }
  std::vector<size_t> max_lens(ts[0].size(), 0);
  for (auto &t : ts) {
    for (size_t i = 0; i < t.size(); i++) {
      max_lens[i] = std::max(max_lens[i], t[i].size());
    }
  }
  for (size_t i = 0; i < ts[0].size(); i++) {
    result += "+";
    for (size_t j = 0; j < max_lens[i] + 2; j++) {
      result += "-";
    }
  }
  result += "+\n";
  for (auto &t : ts) {
    result += "| ";
    for (size_t i = 0; i < t.size(); i++) {
      result += t[i].to_string();
      for (size_t j = 0; j < max_lens[i] - t[i].size() + 1; j++) {
        result += " ";
      }
      result += "| ";
    }
    result += "\n";
  }
  for (size_t i = 0; i < ts[0].size(); i++) {
    result += "+";
    for (size_t j = 0; j < max_lens[i] + 2; j++) {
      result += "-";
    }
  }
  result += "+\n";
  return result;
}

std::string res;

void result_cb_2(std::vector<Tuple> &ts, RC rc) {
  auto str = result2str(ts, rc);
  std::cout << "res str"  << str;
  res = std::move(str);
}

std::string handle_error2str(const std::string& query, RC rc) {
  std::stringstream ss;
  ss << "\033[31m"
            << "Error: \n"
            << "        query: '" << query << "' " << rc2str(rc) << "\033[0m" << std::endl;
  return ss.str();
}

void handle_error(const std::string &query, RC rc)
{
  std::cout << "\033[31m"
            << "Error: \n"
            << "        query: '" << query << "' " << rc2str(rc) << "\033[0m" << std::endl;
}

std::string exec_query(std::string query)
{
  std::string q = query;
  auto s = std::unique_ptr<Stage>(new ParsingStage{});
  ParsingStage *p = (ParsingStage *)s.get();
  p->init(std::move(query));
  RC rc = s->do_request();
  if (!rc_success(rc)) {
    return handle_error2str(q, rc);
  }
  return "";
}


void handle_client(int client_fd) {
  char buffer[1024] = {0};
  bool c_quit = false;

  while (!c_quit) {
    bzero(buffer, 1024);
    int n = read(client_fd, buffer, 1024);
    if (n < 0) {
      std::cerr << "Error reading from socket" << std::endl;
      break;
    }

    std::string query{buffer};
    std::cout << "query : " << query << std::endl;
    if (query == ".quit\n") break;

    res.clear();
    exec_query(query);

    int m = write(client_fd, res.data(), res.size());
    if (m < 0) {
      std::cerr << "Error writing to socket" << std::endl;
      break;
    }
  }
  close(client_fd);
}

int main(int argc, char *argv[]) {
  if (!std::filesystem::exists(".tadis")) {
    std::filesystem::create_directory(".tadis");
  }
  Catalog::catalog().init(".tadis");
  ResultStage::defaultCb = result_cb_2;
  
  
  int server_fd, client_fd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Error creating socket" << std::endl;
    return 1;
  }

  std::memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(8082);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    std::cerr << "Error binding socket" << std::endl;
    return 1;
  }

  if (listen(server_fd, 5) < 0) {
    std::cerr << "Error listening on socket" << std::endl;
    return 1;
  }

  while (true) {
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
      std::cerr << "Error accepting client" << std::endl;
      continue;
    }
    handle_client(client_fd);
  }

  close(server_fd);
  return 0;
}

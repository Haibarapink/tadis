#pragma once
#include "json/json_parser.hpp"
#include "json/json_value.hpp"
#include "json/json_writer.hpp"

#include <fstream>

class JsonFileRWer {
public:
  bool read(const std::string& name, pson::Value& val);
  bool write(const std::string& name, pson::Value& val);
};

inline bool JsonFileRWer::read(const std::string &name, pson::Value &val)
{
  std::fstream fs;
  std::string data;

  fs.open(name, std::ios_base::in);
  assert(fs.is_open());
  while (fs.good()) {
    char ch;
    fs >> ch;
    data.push_back(ch);
  }
  fs.close();

  pson::Parser p{data};
  bool ok = p.Parse(val);
  return ok;
}

inline bool JsonFileRWer::write(const std::string &name, pson::Value &val)
{
  std::fstream fs;
  fs.open(name, std::ios_base::out);

  if (!fs.is_open()) {
    return false;
  }

  auto data = val.print();
  fs << data;
  fs.flush();
  fs.close();

  return true;
}
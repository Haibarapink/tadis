/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-05 19:39:35
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-15 00:24:11
 * @FilePath: /tadis/src/common/logger.hpp
 * @Description: A logger that was writen very simple.
 */
#pragma once

#include <thread>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

class LogRecord
{
public:
  friend class Logger;

  LogRecord(std::string_view loglevel)
  {
    ss_ << "[" << loglevel << "]" ;
  }

  auto ss() -> std::stringstream &{
    return ss_;
  }

  ~LogRecord();
private:
  std::stringstream ss_;
};

class Logger
{
public:
  Logger() = default;
  ~Logger() {
    fs_.flush();
  }

  void init(std::string_view filename)
  {
    fs_.open(filename.data(), std::ios::out | std::ios::app);
  }

  void write_record(LogRecord& rec)
  {
    fs_ << rec.ss_.str() << "\n";
  }

  static Logger& logger()
  {
    if (!l.fs_.is_open()) {
      l.init("default.log");
    }
    return l;
  }

  static void init_logger(std::string_view filename)
  {
    Logger::l.init(filename);
  }
private:
  static Logger l;
  std::fstream fs_;
};

Logger Logger::l;

LogRecord::~LogRecord() {
  auto && logger = Logger::logger();
  logger.write_record(*this);
}

#define LOG(level) LogRecord(level).ss() << "[" << __FILE__ << ":" << __LINE__<< "]"
#define LOG_DEBUG \
            LOG("DEBUG")
#define LOG_WARN \
            LOG("WARN")


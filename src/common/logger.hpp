#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

// cmake open debug
#ifdef DEBUG
#define LOG_OPEN 1
#endif

namespace logger {

class LogRecord {
public:
  friend class Logger;
  LogRecord() = default;
  ~LogRecord();

#ifdef LOG_OPEN
  LogRecord(std::string_view loglevel)
  {
    ss_ << "[" << loglevel << "]";
  }
  template <typename T>
  auto operator<<(T t) -> LogRecord &
  {
    ss_ << t;
    return *this;
  }

  std::stringstream ss_;
#else
  LogRecord(std::string_view loglevel)
  {}
  template <typename T>
  auto operator<<(T t) -> LogRecord &
  {
    return *this;
  }
#endif
};

inline LogRecord::~LogRecord()
{
#ifdef LOG_OPEN
  std::cout << ss_.str() << std::endl;
#endif
}

#define LOG(level) logger::LogRecord(level) << "[" << __FILE__ << ":" << __LINE__ << "]"
#define LOG_DEBUG LOG("DEBUG")
#define LOG_WARN LOG("WARN")
}  // namespace logger
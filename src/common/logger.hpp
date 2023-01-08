/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-05 19:39:35
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-08 11:43:33
 * @FilePath: /tadis/src/common/logger.hpp
 * @Description: 对 boost log的包装
 */

#pragma once

#include <boost/log/core.hpp>
#include <boost/log/expressions/filter.hpp>
#include <boost/log/keywords/severity.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

// inline void set_filter(const logging::trivial::severity_level &level)
// {
//   // TODO doc bug
// }

inline void add_file_log(const std::string &file)
{
  logging::add_file_log(file);
}

#define BOOST_LOG_WRAP(level) BOOST_LOG_TRIVIAL(level) << "[ " << __FILE__ << ":" << __LINE__ << "]"

#define LOG_TRACE BOOST_LOG_WRAP(trace)
#define LOG_DEBUG BOOST_LOG_WRAP(debug)
#define LOG_INFO BOOST_LOG_WRAP(info)
#define LOG_WARN BOOST_LOG_WRAP(warning)
#define LOG_ERROR BOOST_LOG_WRAP(error)
#define LOG_FATAL BOOST_LOG_WRAP(fatal)
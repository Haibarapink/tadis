#pragma once
/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-14 00:03:19
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-14 23:17:55
 */
#include "common/rc.hpp"
#include "storage/tuple.hpp"
#include "common/logger.hpp"
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/value.hpp>
#include <cassert>

template <typename T>
inline boost::json::value to_json(T &t)
{
  return t.to_json();
}

template <typename T>
inline RC from_json(T &t, const boost::json::value &v)
{
  return t.from_json(v);
}

boost::json::value TupleCellMeta::to_json()
{
  boost::json::object obj;
  obj.emplace("name", name_);
  obj.emplace("type", static_cast<int>(type_));
  obj.emplace("len", len_);

  auto v = boost::json::value(std::move(obj));

  return v;
}

RC TupleCellMeta::from_json(const boost::json::value &v)
{
  if (!v.is_object()) {
    LOG_DEBUG << "v is not a json object";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  auto &&obj = v.as_object();

  auto &&name_v = obj.at("name");

  if (!name_v.is_string()) {
    LOG_DEBUG << "name_v's type isn't string";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  name_ = name_v.as_string();

  auto &&type_v = obj.at("type");
  if (!type_v.is_int64()) {
    LOG_DEBUG << "type_v's type isn't int64";
    return RC::JSON_DESERIALIZATION_ERROR;
  }

  if (type_v.as_int64() > static_cast<int>(TupleCellType::UNKNOW) ||
      type_v.as_int64() < static_cast<int>(TupleCellType::FLOAT)) {
    LOG_DEBUG << "type_v " << type_v.as_int64() << " out of range of enum class";
    return RC::JSON_DESERIALIZATION_ERROR;
  }

  type_ = static_cast<TupleCellType>(type_v.as_int64());

  auto &&len_v = obj.at("len");
  if (!len_v.is_uint64()) {
    LOG_DEBUG << "len_v's type isn't uint64";
    return RC::JSON_DESERIALIZATION_ERROR;
  }

  len_ = len_v.as_uint64();

  return RC::SUCCESS;
}

boost::json::value TupleMeta::to_json()
{
  boost::json::object obj;
  boost::json::array cells;
  for (auto &cell : cells_) {
    cells.push_back(cell.to_json());
  }
  obj.emplace("cells", std::move(cells));
  auto v = boost::json::value(std::move(obj));

  return v;
}

RC TupleMeta::from_json(const boost::json::value &v)
{
  if (!v.is_object()) {
    LOG_DEBUG << "v isn't a json object";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  auto &&obj = v.as_object();

  auto &&cells_v = obj.at("cells");
  if (!cells_v.is_array()) {
    LOG_DEBUG << "cells_v isn't a json array";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  auto &&cells = cells_v.as_array();
  for (auto &&cell : cells) {
    TupleCellMeta m;
    auto rc = m.from_json(cell);
    if (!rc_success(rc)) {
      return rc;
    }
    cells_.emplace_back(std::move(m));
  }
  return RC::SUCCESS;
}

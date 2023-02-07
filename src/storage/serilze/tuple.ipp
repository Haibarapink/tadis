#pragma once
/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-14 00:03:19
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 04:30:14
 */
#include "common/rc.hpp"
#include "storage/tuple.hpp"
#include "common/logger.hpp"
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/value.hpp>
#include <cassert>

template <typename T>
inline boost::json::object to_json(T &t)
{
  return t.to_json();
}

template <typename T>
inline RC from_json(T &t, const boost::json::value &v)
{
  return t.from_json(v);
}

inline boost::json::object TupleCellMeta::to_json()
{
  boost::json::object obj;
  obj.emplace("name", name_);
  obj.emplace("type", static_cast<int>(type_));
  obj.emplace("len", len_);
  obj.emplace("visible", visible_);

  return obj;
}

inline RC TupleCellMeta::from_json(const boost::json::value &v)
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
  if (!len_v.is_int64()) {
    LOG_DEBUG << "len_v's type isn't uint64";
    return RC::JSON_DESERIALIZATION_ERROR;
  }

  len_ = len_v.as_int64();

  auto &&visible_v = obj.at("visible");
  if (!visible_v.is_bool()) {
    LOG_DEBUG << "visible_v's type isn't bool";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  visible_ = visible_v.as_bool();

  return RC::SUCCESS;
}

inline boost::json::value TupleMeta::to_json()
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

inline RC TupleMeta::from_json(const boost::json::value &v)
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

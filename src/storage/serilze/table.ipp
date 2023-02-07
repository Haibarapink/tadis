/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-15 10:34:42
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-02-08 04:29:17
 * @FilePath: /tadis/src/storage/serilze/table.ipp
 * @Description: table's serilzation
 */
#pragma once

#include "common/logger.hpp"
#include "common/rc.hpp"
#include "storage/table.hpp"
#include <boost/json/object.hpp>

inline RC TableMeta::from_json(const boost::json::value &v)
{

  if (!v.is_object()) {
    return RC::JSON_DESERIALIZATION_ERROR;
  }

  auto &&obj = v.as_object();

  auto name_iter = obj.find("name");
  if (name_iter == obj.end()) {
    LOG_DEBUG << "can't find name_";
    return RC::JSON_DESERIALIZATION_ERROR;
  }
  auto &&name_v = name_iter->value();
  if (!name_v.is_string()) {
    LOG_DEBUG << "name_v's type isn't string";
    return RC::JSON_DESERIALIZATION_ERROR;
  }

  name_ = name_v.as_string();

  auto meta_iter = obj.find("meta");

  if (meta_iter == obj.end()) {
    LOG_DEBUG << "can't find meta_";
    return RC::OUT_OF_RANGE;
  }

  auto &&meta_v = meta_iter->value();
  if (auto rc = meta_.from_json(meta_v); !rc_success(rc)) {
    return rc;
  }

  return RC::SUCCESS;
}

inline boost::json::object TableMeta::to_json()
{
  boost::json::object obj;
  obj.emplace("name", name_);
  obj.emplace("meta", meta_.to_json());
  return obj;
}
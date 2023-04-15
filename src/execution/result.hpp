#pragma once

#include <memory>
#include "catalog/tuple.hpp"
#include "common/rc.hpp"
#include "common/expect.hpp"

#include <vector>
#include <string>
#include <variant>
#include <optional>

class QueryResult {
public:
  QueryResult(RC rc = RC::SUCCESS, std::string msg = "") : msg_(msg), rc_(rc)
  {}

  QueryResult(const QueryResult &) = delete;
  QueryResult &operator=(const QueryResult &) = delete;

  QueryType type_;
  std::string msg_;
  RC rc_;
  std::vector<Tuple> tuples_;
  std::unique_ptr<Schema> output_schema_;
};

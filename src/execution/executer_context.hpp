#pragma once

#include "catalog/catalog.hpp"
#include "catalog/schema.hpp"
#include "execution/result.hpp"

class ExecuterContext {
public:
  ExecuterContext(Catalog* catalog) : catalog_(catalog){}
  auto catalog() const { return catalog_; }

  QueryResult result_;
private:
  Catalog* catalog_ = nullptr;
};
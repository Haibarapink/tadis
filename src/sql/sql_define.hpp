/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-06 11:50:03
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-06 13:17:46
 * @FilePath: /tadis/src/sql/sql_define.hpp
 * @Description: sql的定义，比如select ast, insert ast等等
 */

#pragma once

#include <vector>
#include <variant>

// <Query> ::= SELECT <SelList> FROM <FromList> WHERE <Condition>
// <SelList> ::= <Attribute> , SelList>
// <SelList> ::= <Attribute>

template<typename... Queries>


class Select {
public:
private:
  
};
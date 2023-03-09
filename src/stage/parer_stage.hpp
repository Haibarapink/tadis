/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-03-09 14:59:49
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-03-09 16:09:16
 * @FilePath: /tadis/src/stage/parsing_stage.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 */
#pragma once

#include "sql/parser/parser.hpp"
#include "stage/stage.hpp"
#include "stage/exec_stage.hpp"

class ParsingStage : public Stage {
public:
  ParsingStage()
  {
    name_ = "ParsingStage";
  }

  void init(std::string query_str)
  {
    query_ = std::move(query_str);
  }

  RC handle_event()
  {
    Parser<std::string_view> parser{query_};

    RC rc = parser.parse();
    if (rc != RC::SUCCESS) {
      return rc;
    }

    return rc;
  }

private:
  std::string query_;
};
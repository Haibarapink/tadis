/*
 * @Author: pink haibarapink@gmail.com
 * @Date: 2023-01-15 11:15:17
 * @LastEditors: pink haibarapink@gmail.com
 * @LastEditTime: 2023-01-15 11:18:27
 * @FilePath: /tadis/src/stage/stage.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 */
#pragma once

#include <boost/core/noncopyable.hpp>
class Stage : public boost::noncopyable {
public:
  virtual void do_request();

private:
};
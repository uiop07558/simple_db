#pragma once

#include "../page/page.hpp"

class Pager {
 public:
  virtual Page getPage(pageptr_t ptr) = 0; // get page by its id
  virtual pageptr_t addPage(const Page& page) = 0; // add new page
  virtual void delPage(pageptr_t ptr) = 0; // delete page by its id
};
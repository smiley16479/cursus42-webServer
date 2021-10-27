#ifndef _HEADER_HANDLER_HPP_
#define _HEADER_HANDLER_HPP_
#include <string>
#include <fstream>
#include <iostream>
#include "color.hpp"
using namespace std;

class header_handler
{
private:
  // map ou vector ?
public:
  header_handler(/* args */);
  ~header_handler();
  void reader(char *);
};

#endif
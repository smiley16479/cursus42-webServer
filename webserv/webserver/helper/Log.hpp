#ifndef LOG_HPP
# define LOG_HPP

#include <iostream>


void log(const std::string& error);
void debug_log(const std::string& msg);
void error_log(const std::string& error);

#endif
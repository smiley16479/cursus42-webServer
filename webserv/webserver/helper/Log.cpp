#include "./Log.hpp"

void log(const std::string& msg)
{
	std::cerr << "\033[1m\033[34m[WebServ log] \033[0m: " << msg << std::endl;
}

void debug_log(const std::string& msg)
{
	std::cerr << "\033[1m\033[30m[WebServ debug] \033[0m: " << msg << std::endl;
}

void error_log(const std::string& error)
{
	std::cerr << "\033[1m\033[31m[WebServ ERROR] \033[0m: " << error << std::endl;
}
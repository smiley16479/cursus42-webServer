#include "header_handler.hpp"
#include <sstream>

header_handler::header_handler(/* args */)
{

}

header_handler::~header_handler()
{
}

void header_handler::reader(char *str)
{
	string buf = "hello";
	std::stringstream ss(str);
	while (std::getline(ss, buf))
		cout << RED << buf << RESET << endl;
}
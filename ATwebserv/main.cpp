#include <iostream>
#include <cstdlib>
#include "config_checker.hpp"
#include "server.hpp"
#include "color.hpp"

using namespace std;


int main(int ac, char const *av[])
{
	// if (ac != 2)
		// exit(EXIT_FAILURE);
	av[1] = "./configuration_files/valid/default.conf";
	config_checker confCheck;
	try
	{
		confCheck.check_conFile(av[1]);
		server webserv(&confCheck._si);
		webserv.display_server(); // A utilité de check : peut être enlevé...
		webserv.run();
	}
	catch(const exception& e)
	{
		cerr << RED << e.what() << RESET << endl;
	}
	catch (...)
	{
		cout << "catch (...)\n" << endl;
	}
	
	return 0;
}

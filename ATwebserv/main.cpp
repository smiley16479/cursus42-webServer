#include <iostream>
#include <cstdlib>
#include "config_checker.hpp"
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
		// throw (Tmyex);
		confCheck.check_conFile(av[1]);
		// testClass2 t2;
		// t2.testThrow2();
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


/* // using standard exceptions
#include <iostream>
#include <exception>
using namespace std;

class myexception: public exception
{
  virtual const char* what() const throw()
  {
    return "My exception happened";
  }
} myex;

int main () {
  try
  {
    throw myex;
  }
  catch (exception& e)
  {
    cout << e.what() << '\n';
  }
  return 0;
} */
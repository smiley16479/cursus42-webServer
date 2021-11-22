#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

#include <iostream>
#include <cstdlib>
#include <vector>
//#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>

# define CGI "files/cgi/php-cgi"
# define SCRIPT "files/scripts/hello.php"

class	cgi_handler	{
//	private:
	public:
		std::string	handler;
		std::string	script;
		std::vector<char*>	args;

	public:
		cgi_handler();
		cgi_handler(const cgi_handler& other);
		~cgi_handler();

		cgi_handler&	operator=(const cgi_handler& other);
};

#endif

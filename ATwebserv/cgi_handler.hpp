#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

# include <iostream>
# include <cstdlib>
# include <vector>
# include <fstream>
# include <sstream>
# include <algorithm>
//# include <pthread.h>
# include <sys/wait.h>
# include <unistd.h>
# include <cstring>
# include <map>
//# include <stringstram>

# define CRASH_FORK	-1
# define CRASH_PIPE	-1

# define CGI "files/cgi/php-cgi"
//# define SCRIPT "files/scripts/hello.php"
# define SCRIPT "files/scripts/variables.php"

bool	is_cgi(std::vector<std::string>& query);
void	go_cgi(std::map<std::string, std::vector<std::string> >& mp);


class	cgi_handler	{
//	private:
	public:
		std::string	handler;
		std::string	script;
		std::vector<std::string>	args;

	public:
		cgi_handler();
		cgi_handler(std::vector<std::string>& vec);
		cgi_handler(const cgi_handler& other);
		~cgi_handler();

		cgi_handler&	operator=(const cgi_handler& other);
};

#endif

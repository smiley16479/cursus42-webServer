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

# include "struct_webserv.hpp"

# define CGI_ENV	{	\
	"SERVER_SOFTWARE\n"	\
	"SERVER_NAME\n"	\
	"GATEWAY_INTERFACE\n"	\
	"SERVER_PROTOCOL\n"	\
	"SERVER_PORT\n"	\
	"REQUEST_METHOD\n"	\
	"PATH_INFO\n"	\
	"PATH_TRANSLATED\n"	\
	"SCRIPT_NAME\n"	\
	"QUERY_STRING\n"	\
	"REMOTE_HOST\n"	\
	"REMOTE_ADDR\n"	\
	"AUTH_TYPE\n"	\
	"REMOTE_USER\n"	\
	"REMOTE_IDENT\n"	\
	"CONTENT_TYPE\n"	\
	"CONTENT_LENGTH\n"	\
	"HTTP_ACCEPT_LANGUAGE\n"	\
	"HTTP_USER_AGENT\n"	\
	"HTTP_COOKIE\n"	\
	"HTTP_REFERER\n"	\
	}

# define CRASH_FORK	-1
# define CRASH_PIPE	-1

# define CGI "files/cgi/php-cgi"
//# define SCRIPT "files/scripts/hello.php"
# define SCRIPT "files/scripts/variables.php"

bool	is_cgi(std::vector<std::string>& query);
void	go_cgi(std::map<std::string, std::vector<std::string> >& mp, std::vector<server_info>& serv);


class	cgi_handler	{
//	private:
	public:
		std::string	handler;
		std::string	script;
		std::vector<std::string>	args;

	public:
		cgi_handler();
		cgi_handler(std::map<std::string, std::vector<std::string> >& mp, std::vector<server_info>& serv);
		cgi_handler(const cgi_handler& other);
		~cgi_handler();

		cgi_handler&	operator=(const cgi_handler& other);
};

#endif

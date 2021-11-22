#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

# include <iostream>
# include <cstdlib>
# include <vector>
//# include <pthread.h>
# include <sys/wait.h>
# include <unistd.h>
//# include <stringstram>
# include <fstream>

# define CRASH_FORK	-1
# define CRASH_PIPE	-1

# define CGI "files/cgi/php-cgi"
# define SCRIPT "files/scripts/hello.php"

# define HEADER	"\
<!doctype html>\n\
<html>\n\
  <head>\n\
    <title>This is the title of the webpage!</title>\n\
  </head>\n\
  	<body>\n"


# define FOOTER "</body>\n\
	</html>"

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

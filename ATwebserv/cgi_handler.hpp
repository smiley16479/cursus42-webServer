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
#include <fcntl.h>
//# include <stringstram>

# include "struct_webserv.hpp"
# include "request_handler.hpp"

class request_handler;

class cgi_handler
{
private:
	/* data */
	request_handler &obj;
public:
	cgi_handler(request_handler &obj);
	~cgi_handler();

		/* FUNCTION DE CGI */

	int cgi_input();
	int cgi_output();
	char**setCGIEnv();
	void clean_body();
	int handle_cgi(void);
	bool	is_cgi(); // paramètres enlevés et créés à l'intérieur des fonctions
	int	launch_cgi(); // paramètres enlevés et créés à l'intérieur des fonctions
};

#endif

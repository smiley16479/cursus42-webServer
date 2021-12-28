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

/*
x	"SERVER_SOFTWARE=";
x	"SERVER_NAME=";
x	"GATEWAY_INTERFACE=";
x	"SERVER_PROTOCOL=";
x	"SERVER_PORT=";
x	"REQUEST_METHOD=";
x	"PATH_INFO=";
x	"PATH_TRANSLATED=";
x	"SCRIPT_NAME=";
x	"QUERY_STRING=";
x	"REMOTE_HOST=";
x	"REMOTE_ADDR=";
x	"AUTH_TYPE=";
x	"REMOTE_USER=";
x	"REMOTE_IDENT=";
x	"CONTENT_TYPE=";
x	"CONTENT_LENGTH=";
x   "HTTP_ACCEPT";
x	"HTTP_ACCEPT_LANGUAGE=";
x	"HTTP_USER_AGENT=";
x	"HTTP_COOKIE=";
x	"HTTP_REFERER=";
*/

# define CRASH_FORK	-1
# define CRASH_PIPE	-1

# define CGI "files/cgi/php-cgi"
//# define CGI_MODE "-f"
//# define SCRIPT "files/scripts/hello.php"
# define SCRIPT "files/scripts/variables.php"

bool	is_cgi(std::vector<std::string>& query, std::vector<std::string>& extensions);
int	go_cgi(std::vector<std::string>& post_args, std::vector<std::string>& env);
size_t	getcLen(std::vector<std::string>& env);


#endif

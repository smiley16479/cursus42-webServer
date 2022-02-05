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

//# define CGI "files/cgi/php-cgi"
//# define CGI_MODE "-f"
//# define SCRIPT "files/scripts/hello.php"
//# define SCRIPT "files/scripts/variables.php"
//	e_path[0] = cgi_path.c_str();
//	e_path[1] = (char*)CGI_MODE;
//	e_path[2] = NULL;

int	is_cgi(std::vector<std::string>& query, std::vector<std::string>& extensions);
int	go_cgi(int (*rfd)[2], std::string cgi_path, std::vector<std::string>& env);
size_t	getcLen(std::vector<std::string>& env);

#endif

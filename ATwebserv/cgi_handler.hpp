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

//# define CGI "files/cgi/php-cgi"
//# define CGI_MODE "-f"
//# define SCRIPT "files/scripts/hello.php"
# define SCRIPT "files/scripts/variables.php"

bool	is_cgi(std::vector<std::string>& query, std::vector<std::string>& extensions);
int	go_cgi(std::string cgi_path, std::vector<std::string>& post_args, std::vector<std::string>& env);
size_t	getcLen(std::vector<std::string>& env);


#endif

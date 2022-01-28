#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

# include <iostream>
# include <cstdlib>
# include <vector>
# include <fstream>
# include <sstream>
# include <algorithm>
# include <sys/wait.h>
# include <unistd.h>
# include <cstring>
# include <map>
#include <fcntl.h>
#include <arpa/inet.h>

# include "struct_webserv.hpp"

int	is_cgi(std::vector<std::string>& query, std::vector<std::string>& extensions);
int	go_cgi(int (*rfd)[2], std::string cgi_path, std::vector<std::string>& env);
size_t	getcLen(std::vector<std::string>& env);
int	go_cgi_fd(int (*rfd)[2], std::string cgi_path, std::vector<std::string>& env, std::string& body);


#endif

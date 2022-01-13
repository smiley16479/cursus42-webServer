#ifndef CLIENT_HANDLER_HPP
# define CLIENT_HANDLER_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <netdb.h>		// pour struct sockaddr_in
#include <unistd.h>		// pour close
#include <sys/epoll.h>
#include "struct_webserv.hpp"
#include "client_info.hpp"
#include "server.hpp"

#include <vector>
#include <cstdlib>
#include <fcntl.h>
#include <cstring>
#include <cstdio>

#include <string>

using namespace std;

class client_handler
{
private:
	vector <client_info> clients;

public:
	client_handler(/* args */);
	~client_handler();

	client_handler& operator=(const client_handler& other)
	{ (void)other; return *this;}

	bool is_request_fulfilled(int);
	void remove(struct_epoll& _epoll, int i); // REMOVE A CLIENT
	void remove_fd(struct_epoll& _epoll, int fd); // REMOVE A CLIENT
	void add(struct_epoll& _epoll, int time_out, int i); // ADD A CLIENT
	void check_all_timeout();

	/* Function rubrique : getter setter */
	void clear(int client_fd); // CLEAR CLIENT CONTENT
	void rqst_append(int , char *, int); // APPEND DIFFERENT REQUEST CHUNK
	string get_rqst(int); // RETURN FINAL REQUEST

	/* FUNCTION SECONDAIRE : UTILITAIRES */

	client_info*	get_info(int fd);

	std::vector<client_info>*	data() { return (&this->clients); };
	bool is_post_rqst_fulfilled(client_info& client);
	bool is_chunked_rqst_fulfilled(client_info& client);
	int					redir_cgi(client_info& client);
	int					redir_read(client_info& client);
	int					redir_write(client_info& client);
	std::vector<int>	handle_pendings(struct_epoll& _epoll);
	void	fill_resp(int fd, std::string& base);
	int		chunked_rqst(struct_epoll& _epoll, int fd);
	int		chunked_resp(int fd);
	void 	time_reset(int time_out, int fd); // Reset client fd
	int		no_chunk(int fd);
};

#endif

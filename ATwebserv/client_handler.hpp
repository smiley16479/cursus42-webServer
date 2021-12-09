#include <map>
#include <string>
#include <netdb.h>		// pour struct sockaddr_in
#include <unistd.h>		// pour close
#include <iostream>
#include <sys/epoll.h>
#include "struct_webserv.hpp"

#include <fcntl.h>

#include <string>

using namespace std;

class client_handler
{
private:
	map <int, client_info> clients;

public:
	client_handler(/* args */);
	~client_handler();
	bool is_request_fulfilled(int);
	void remove(struct_epoll& _epoll, int i); // REMOVE A CLIENT
	void add(struct_epoll& _epoll, int time_out, int i); // ADD A CLIENT
	void check_all_timeout(struct_epoll& _epoll);

	/* Function rubrique : getter setter */
	void clear(int client_fd); // CLEAR CLIENT CONTENT
	void rqst_append(int , char *); // APPEND DIFFERENT REQUEST CHUNK
	string get_rqst(int); // RETURN FINAL REQUEST

	/* FUNCTION SECONDAIRE : UTILITAIRES */

	bool is_post_rqst_fulfilled(int client_fd);
};

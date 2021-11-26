#include <map>
#include <string>
#include <netdb.h>		// pour struct sockaddr_in
#include <unistd.h>		// pour close
#include <iostream>
#include <sys/epoll.h>
#include "struct_webserv.hpp"

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
	void clear(int client_fd); // CLEAR CLIENT CONTENT
	void add(struct_epoll& _epoll, int time_out, int i); // ADD A CLIENT
	void rqst_append(int , char *); // APPEND DIFFERENT REQUEST CHUNK
	string get_rqst(int); // RETURN FINAL REQUEST
	void check_all_timeout(void);

};



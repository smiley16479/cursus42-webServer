#include <map>
#include <string>
#include <netdb.h>		// pour struct sockaddr_in
#include <unistd.h>		// pour close
#include <iostream>
#include <fstream>
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
	bool is_POST_request_fulfilled(int client_fd);
		bool is_POST_chunk_fulfilled(int client_fd);
		bool is_POST_regular_fulfilled(int client_fd);
	void remove(struct_epoll& _epoll, int i); // REMOVE A CLIENT
	void add(struct_epoll& _epoll, int time_out, int i); // ADD A CLIENT
	void check_all_timeout(struct_epoll& _epoll);

	/* FUNCTION RUBRIQUE : GETTER SETTER */
	void clear(int client_fd); // CLEAR CLIENT CONTENT
	void rqst_append(int client_id, char *str, int byte_recved); // APPEND DIFFERENT REQUEST CHUNK
	client_info& get_rqst(int); // RETURN FINAL REQUEST

	/* FUNCTION RUBRIQUE : UTILITAIRE */

	// recherche uniquement dans une sous-portion de la string
	size_t portion_search(string haystack, string needle, size_t from = 0, size_t to = string::npos);

};



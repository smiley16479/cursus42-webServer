#ifndef _STRUCT_WEBSERV_HPP_
# define _STRUCT_WEBSERV_HPP_
# include <sys/epoll.h>	// for epoll_create1(), epoll_ctl(), struct epoll_event 
# include <string>
# include <iostream>
# include <vector>
# include <map>

enum	e_mode	{
//describes client status
	RECV,//reads on communication socket
	CHUNKED,
	COMPUTE,//checks request fulfillment and computes response
	WRITE,//writes to a local file
	READ,//reads from a local file
	CGI_IN,//reads on cgi pipe end
	CGI_OUT,//reads on cgi pipe end
	SEND,//sends on communication socket
	NONE//indicates that current action doesn't necessitate a state change
};

enum	e_cmode	{
	NO_CHUNK,
	CHUNK_PARSING_REQ,
	CHUNK_INCOMPLETE,
	CHUNK_COMPLETE,
	TRANSMISSION_OVER,
	TRANSMIT_100,
	BAD_REQUEST
};

enum	e_cflags	{
	ZERO,
	EXPECT_CHUNKS,
	EXPECT_LINE_NUM,
	EXPECT_NUM,
	EXPECT_NL_NUM,
	EXPECT_LEN,
	EXPECT_NL_LEN,
	EXPECT_END
};


enum	e_rqmode	{
	NORMAL,
	MULTIPART,
	CLEN
};

//# define MAX_LEN 20971520
//# define MAX_LEN 8192

# define MAX_EVENTS 1000

struct locati_info {
	std::string	location;
	std::string auth_basic;							// ?
	std::string auth_user_file;						// fichiers autorisés à l'utilisateur
	std::string autoindex;							// présentation du fileSystem => val (on/off)
	std::string index;								// fichier servi par défault
	std::string max_file_size;						// Sets the maximum allowed size of the client request body. If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client. Please be aware that browsers cannot correctly display this error. Setting size to 0 disables checking of client request body size. 
	std::string return_directive;					// redirection vers une autre location
	std::string root;								// dossier racine
	std::vector<std::string> allowed_method;		// méthodes (GET, POST, etc) permises
	std::string cgi_path;							// Path de l'executable chargé des cgi
	std::vector<std::string> cgi_file_types;		// type de file gérées pour les cgi
	std::string upload_path;							// Path du dossier stockant les upload
	std::vector<std::string> retour;				// directive return
};

struct server_info {
	int socket;
	std::string port;
	std::string host;
	std::string server_name;
	std::string error_page;
	std::string max_file_size;						// Sets the maximum allowed size of the client request body. If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client. Please be aware that browsers cannot correctly display this error. Setting size to 0 disables checking of client request body size. 
	std::string time_out;
	std::vector<locati_info> location;
};

struct struct_epoll
{
    int _epoll_fd;
    int _event_count;
	epoll_event _event;
	epoll_event _events[MAX_EVENTS];
};


/* 
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

// trim from start
static inline std::string &ltrim(std::string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(),
						std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(),
						std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
		return ltrim(rtrim(s));
}
 */

#endif

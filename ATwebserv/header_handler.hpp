#ifndef _HEADER_HANDLER_HPP_
#define _HEADER_HANDLER_HPP_
#include <limits.h> /* PATH_MAX */
#include <string>
// #include <string.h> // bzero sb = {0} à la place
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>
#include <exception>
#include <map>
#include <vector>
#include "struct_webserv.hpp"
#include "color.hpp"
using namespace std;

class header_handler
{

	typedef std::map<string, string> error_it;
private:
    std::vector<server_info> &			_si; // server_info
	int									_s_id; // server id <- quel server doit répondre à la requete actuelle : '_s_id' est l'index de '_si'
	std::map<string, vector<string> >	_hrx; // received header_info (requets header info)
	std::map<string, vector<string> >	_htx; // response_Header
	std::map<string, string>			_status; // Response_Status_Msg -> "404" "Not found" etc.
	std::string							_response; // response_content (response_header + body) OU SLMT BODY ?


	/* COPY DES ATTRIBUTS DE QUING_LY */
/*
	//status
	int															_status;
	std::map<int, std::string>									_status_phrases;
	bool														_write_to_file;
	bool														_read_from_file;
	bool														_write_to_browser;
	int															_bytes_written;
	int															_bytes_read;

	//Headers
	int															_max_file_size;
	int															_content_length;
	std::string													_content_type;
	std::string													_content_language;
	std::string													_content_location;
	std::vector<std::string>									_allow;
	std::string													_method;
	std::string													_file_location;
	std::string													_uri_location;
	std::string													_location_block_root;
	std::string													_protocol;
	std::string													_requested_host;
	std::string													_user_agent;
	std::string 												_accept_charset;
	std::string													_accept_language;
	std::string													_authorization;
	std::string													_referer;
	std::string													_body;
	std::vector<std::string>					 				_special_x_header;
	int															_location_index;
	std::string													_auth_basic;
	std::string													_auth_type;

	//Response
	int															_response_size;
	// std::string												_response;
	std::string													_response_file;
	std::string													_additional_cgi_headers;
*/
public:
	header_handler(std::vector<server_info>&);
	~header_handler();
	void reader(const char *);
	void writer(void);

	/* FONCTION ACCESSEUR */

	string &get_response(void);

	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */

private:
	void gen_date(void);
	void gen_startLine(std::map<string, string>::iterator);
	void gen_serv(void);
	void gen_CType(void);
	void gen_CLength(void);
	// void gen_response(void);

	/* FUNCTION SECONDAIRE : UTILITAIRES */

	void set_server_id(void);
	void handle_get_rqst(void);
		void	verify_file_openess(ifstream& fs);
			void 	resolve_path(string &);
				int	location_lookup(string &path, string url, string uri, std::map<std::string, locati_info>& loc);
				int	file_type(string &path, string &uri);

	void handle_post_rqst(void);
	void	handle_cgi(void);

	/* FUNCTION DE DEBUG */
	
	void display(void);
};

#endif

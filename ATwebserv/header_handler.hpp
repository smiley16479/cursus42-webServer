#ifndef _HEADER_HANDLER_HPP_
#define _HEADER_HANDLER_HPP_
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <exception>
#include <map>
#include <vector>
#include "color.hpp"
using namespace std;

class header_handler
{
private:
	// map ou vector ?
	std::map<string, vector<string> > _hi;
	std::string											_response;


	/* COPY DES ATTRIBUTS DE QUING_LY */

	//status
	int															_status;
	std::map<int, std::string>			_status_phrases;
	bool														_write_to_file;
	bool														_read_from_file;
	bool														_write_to_browser;
	int															_bytes_written;
	int															_bytes_read;

	//Headers
	int															_max_file_size;
	int															_content_length;
	std::string											_content_type;
	std::string											_content_language;
	std::string											_content_location;
	std::vector<std::string>				_allow;
	std::string											_method;
	std::string											_file_location;
	std::string											_uri_location;
	std::string											_location_block_root;
	std::string											_protocol;
	std::string											_requested_host;
	std::string											_user_agent;
	std::string 										_accept_charset;
	std::string											_accept_language;
	std::string											_authorization;
	std::string											_referer;
	std::string											_body;
	std::vector<std::string> 				_special_x_header;
	int															_location_index;
	std::string											_auth_basic;
	std::string											_auth_type;

	//Response
	int															_response_size;
	// std::string											_response;
	std::string											_response_file;
	std::string											_additional_cgi_headers;

public:
	header_handler(/* args */);
	~header_handler();
	void reader(char *);
	string& writer(void);
	void header_checker(string&);
	void display(void);

	/* FONCTION ACCESSEUR */

	string &get_response(void);

	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */
private:
	void gen_date(void);
	void gen_startLine(void);
	void gen_serv(void);
	void gen_CType(void);
	void gen_CLenght(void);


};

#endif
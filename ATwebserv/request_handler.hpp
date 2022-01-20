#ifndef _REQUEST_HANDLER_HPP_
#define _REQUEST_HANDLER_HPP_
#include <limits.h> /* PATH_MAX */
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <vector>
#include "struct_webserv.hpp"
#include "color.hpp"
#include <sys/stat.h>	// pour lstat
#include <dirent.h>		// readdir et struct DIR etc.. (plus besoin si on utilise perl pour générer le html)
#include <unistd.h>		// strlen

using namespace std;

class request_handler
{
	typedef int	(request_handler::*t_func)(void);
	typedef std::map<string, string> error_it;
private:
    std::vector<server_info> &			_si; // server_info
	int									_s_id; // server id <- quel server doit répondre à la requete actuelle : '_s_id' est l'index de '_si'
	int									_l_id; // location id <- quel location doit répondre à la requete actuelle : '_l_id' est l'index de 'location' ds '_si'
	std::map<string, vector<string> >	_hrx; // received header_info (requets header info)
	std::map<string, vector<string> >	_htx; // response_Header
	std::map<string, string>			_status; // Response_Status_Msg -> "404" "Not found" etc.
	t_func								_tab[5]; // LookUP table av les fonctions relatives aux types de requete
	std::string							_path; // response_path file
	std::string							_body; // response_content (body)
	std::string							_response; // response_content (response_header + body) -> mis ds client_info dorenavant
	client_info*						_c;


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
	request_handler(std::vector<server_info>&);
	~request_handler();
	void reader(client_info& cl_info);
	void writer(void);

	/* FONCTION ACCESSEUR */

	string &get_response(void);

	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */

private:
	void gen_date(void);
	void gen_startLine(std::map<string, string>::iterator);
	void gen_serv(void);
	void gen_CType(string ext);
	void gen_CLength(void);
	// void gen_response(void);

	/* FUNCTION SECONDAIRE : UTILITAIRES */

	void set_server_id(void);
	int handle_get_rqst(void);
	int handle_put_rqst(void);
		void	verify_file_openess();
			int 	resolve_path();
				void return_directive(vector<locati_info>::reverse_iterator& it);
				bool is_method_allowed(void);
				int	file_type();
					void generate_folder_list();
	size_t check_file_size(void);
	void add_all_field();
	void add_body();
	void clean_url(string& str);
	int handle_post_rqst(void);
		int extract_postMULTI_rqst_body(void);
		int extract_postXFORM_rqst_body(void);
		int extract_postCHUNK_rqst_body(void);

	void	handle_cgi(void);

	/* FUNCTION DE DEBUG */
	
	void display(void);


};

#endif
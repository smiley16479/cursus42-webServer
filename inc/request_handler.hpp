#ifndef _REQUEST_HANDLER_HPP_
#define _REQUEST_HANDLER_HPP_
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
#include <set>
#include <vector>
#include "struct_webserv.hpp"
#include "color.hpp"
#include <dirent.h> // readdir et struct DIR etc.. (plus besoin si on utilise perl pour générer le html)
#include <unistd.h> // pipe pour le script perl
#include <sys/wait.h> // pipe pour le script perl

#include <fcntl.h>

using namespace std;

class request_handler
{

	typedef std::map<string, string> error_it;
private:
    std::vector<server_info> &			_si; // server_info
	int									_s_id; // server id <- quel server doit répondre à la requete actuelle : '_s_id' est l'index de '_si'
	int									_l_id; // location id <- quel location doit répondre à la requete actuelle : '_l_id' est l'index de 'location' ds '_si'
	int									redir_fd[2];
	int									ext_id;
	std::map<string, vector<string> >	_hrx; // received header_info (requets header info)
	std::map<string, vector<string> >	_htx; // response_Header
	std::string							_path; // response_path file
	std::string							_body; // response_content (body)
	std::string							_response; // response_content (response_header + body)

public:
	request_handler(std::vector<server_info>&);
	~request_handler();
	void reader(std::string& rqst);
	int choose_method(void);
	int	gen_resp(void);
	int cgi_writer(void);
	int writer(void);
	void	clean_body();

	/* FONCTION ACCESSEUR */

	string &get_response(void);
	string &get_body(void);
	void set_body(const string &);
	void	fill_redir_fd(int (*loc_fd)[2]);
	void clean(void);

	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */

private:
	void gen_date(void);
	void gen_startLine(size_t ret_code);
	void gen_serv(void);
	void gen_CType(string ext);
	void gen_CLength(void);
	// void gen_response(void);

	/* FUNCTION SECONDAIRE : UTILITAIRES */

	void set_server_id(void);
	void handle_get_rqst(void);
		bool	is_folder(string path);
		bool	is_regular_file(string path);
			int 	resolve_path();
				void	location_lookup();
				int	location_lookup_2();
				int	file_type();
					void generate_folder_list();
	bool is_method_allowed(void);
	void add_all_field();
	int add_body();
	void clean_url(string& str);
	int multipart_form(string& boundary, string& msg);
	int handle_post_rqst(void);
	std::vector<std::string> extract_env(std::map<std::string, std::vector<std::string> >& mp, const server_info& serv);
	void	cgi_var_init();
	int handle_cgi(void);

	/* FUNCTION DE DEBUG */
	
	void display(void);
};

#endif

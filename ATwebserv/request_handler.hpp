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
	std::string							_URLquery; // query string contenu ds l'url
	std::string							_body; // response_content (body)
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
	void gen_CType(void);
	void gen_CLength(int);
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
	string get_file_size(string& path);
	void add_all_field();
	void add_body(int i = 0);
	void clean_url(string& str);
	int handle_post_rqst(void);
		int extract_multi_rqst_body(void);
		int extract_xform_rqst_body(void);
		int extract_chunk_rqst_body(void);
		void write_file(void);
		void does_file_exist(e_rqst_type);
	int handle_head_rqst(void);
	int handle_delete_rqst(void);
	int cgi_input();
	int cgi_output();
	char**setCGIEnv();
	/* FUNCTION DE DEBUG */
	
	void display(void);

	/* ARTHUR FUNCTION : */
// int writer(void);
// int cgi_writer(); //Écrit la requete réponse final des cgi en réponse au client ds _c->_resp INUTILISÉ
// bool is_folder(std::string path); //															INUTILISÉ
// bool is_regular_file(std::string path); //													INUTILISÉ
// void set_body(const string& str); // set _body avec str										INUTILISÉ
// void fill_redir_fd(int (*loc_fd)[2]); // semblais faire changer les fd pour les cgi 			INUTILISÉ
std::vector<std::string> extract_env();
void clean_body();
void cgi_var_init();
int handle_cgi(void);
// void clean(void); // Clear les attribut de request_handler									INUTILISÉ
std::string reverse_resolve_path(std::string &loc_path);
int create_write_resp(std::string &file_path);

// PROVENANT DU FICHIER CLIENT_INFO D'ARTHUR
// void read_handler(void); // ajoute les bout de rqst venant des cgi à la réponse du client 	INUTILISÉ
// void write_handler(void);  // ajoute les bout de rqst reçu destiné aux cgi 					INUTILISÉ
// void send_handler(void); // Envoi une partie du msg puis remet le mode sur RECV 				INUTILISÉ
// void cgi_write_handler(void); // // Ecrit les bout de rqst du client vers l'executable 		INUTILISÉ
void cgi_resp_handler(void);

// PROVENANT DU FICHIER CGI_HANDLER D'ARTHUR

bool	is_cgi(); // paramètres enlevés et créés à l'intérieur des fonctions
int	launch_cgi(); // paramètres enlevés et créés à l'intérieur des fonctions
// size_t	getcLen(std::vector<std::string>& env); Get la longeur du _body contenu ds le header "CONTENT_LENGTH" des cgi je crois... /* INUTILISÉ */

};



#endif
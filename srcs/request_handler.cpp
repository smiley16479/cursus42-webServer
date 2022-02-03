
#include "cgi_handler.hpp"
#include "request_handler.hpp"
#include "status_lines.hpp"
#include "MIME_headers.hpp"

request_handler::request_handler(std::vector<server_info>& server_info) : _si(server_info)
{
	redir_fd[0] = -1;
	redir_fd[1] = -1;
	redir_pid = -1;
}

request_handler::~request_handler()
{
}

int request_handler::reader(std::string& rqst)
{
	size_t	pos;
	string buf_1, buf_2;
	std::string ss_1(rqst);


	std::cout << BLUE "Request Header : " RESET << std::endl;
	std::cout << rqst.substr(0, rqst.find("\r\n\r\n") + 2) << std::endl;
#ifdef _debug_full_
	std::cout << RED "Request Body : " RESET << std::endl;
	std::cout << rqst.substr(rqst.find("\r\n\r\n")) << std::endl;
#endif

#ifdef _debug_
	std::cout << "Parsing request !" << std::endl;
#endif
	pos = rqst.find("\r\n\r\n");
	if (pos != std::string::npos)
	{
#ifdef _debug_
	std::cout << "Extracting header" << std::endl;
#endif
		_hrx["BODY"].clear();
		_hrx["BODY"].push_back(rqst.substr(pos + 4));
		rqst = rqst.substr(0, pos);
	}

#ifdef _debug
std::cout << RED "DANS HEADER READER" RESET "\n" << str << endl;
#endif

	// LECTURE DE LA START_LINE
	if (!ss_1.empty() && (pos = ss_1.find("\r\n")) != string::npos)
	{
#ifdef _debug
		std::cout << "Processing" << std::endl;
#endif
		buf_1 = ss_1.substr(0, pos + 2);
		ss_1 = ss_1.substr(pos + 2);
		std::stringstream ss_2(buf_1);
		while (ss_2 >> buf_1)
			_hrx["A"].push_back(buf_1);
	}

	// LECTURE DU RESTE DE LA REQUETE
	while ((pos = ss_1.find("\r\n")) != string::npos) {
#ifdef _debug
	std::cout << "Processing header" << std::endl;
#endif
		if (ss_1.substr(0, 2) == "\r\n")
			break;
		buf_1 = ss_1.substr(0, pos + 2);
		ss_1 = ss_1.substr(pos + 2);
		std::stringstream ss_2(buf_1);
		while (ss_2 >> buf_2) {
				string index = buf_2;
				while (ss_2 >> buf_2)
					_hrx[index].push_back(buf_2);
		}
	}
	set_server_id();
	if (_s_id == -1)
		return (1);
	return (0);
}

int request_handler::choose_method(void)
{
	int	redir_mode;
	gen_startLine( 200 );
	gen_date();
	gen_serv();

	redir_mode = NONE;
#ifdef _debug
	std::cout << "Choosing adequate method" << std::endl;
#endif
// DÉFINI L'INDEX DE LA LOCATION CONCERNÉE (_l_id) & VÉRIFIE QUE LA MÉTHODE INVOQUÉE Y EST PERMISE
	if (resolve_path())
		gen_allowed();
	else
	{
		if (!_hrx["BODY"].empty()
			&& !_hrx["Transfer-Encoding:"].empty()
			&& _hrx["Transfer-Encoding:"][0] == "chunked")
			_hrx["BODY"][0] = clean_chunk(_hrx["BODY"][0]);
		if ((ext_id = is_cgi(_hrx["A"], _si[_s_id].location[_l_id].cgi_file_types)) != -1)
		{
			if (CGI_MODE == 1)
				redir_mode = handle_cgi_fd();
			else
				redir_mode = handle_cgi();
		}
		else if (_hrx["A"][0] == "POST")
			redir_mode = handle_post_rqst();
		else if (_hrx["A"][0] == "GET")
			handle_get_rqst();
		else if (_hrx["A"][0] == "PUT") {
			redir_mode = handle_put_rqst();
		}
		else if (_hrx["A"][0] == "HEAD") {
			redir_mode = HEAD;
		}
		else if (_hrx["A"][0] == "DELETE") {
			// IF NOT ALLOWED -> 405

			if (_hrx["A"][1][0] == '/')
				_hrx["A"][1].insert(0, ".");
			if( remove( _hrx["A"][1].c_str() ) != 0 ) {
				perror( _hrx["A"][1].c_str() );
				gen_startLine( 404 ); //  IF NOT FOUND -> 404
			}
			else
			{
				std::cout << "File successfully deleted" << std::endl;
				gen_startLine( 204 );
				redir_mode = writer(redir_mode);
				_hrx.clear();
				_htx.clear();
				return (NONE);
			}
		}
	}
	if (redir_mode == NONE || redir_mode == HEAD)
	{
		file_type();
		gen_CType(_path.substr(_path.find_last_of(".") + 1));
		gen_CLength();
		redir_mode = writer(redir_mode);
	}
	_hrx.clear();
	_htx.clear();
	return (redir_mode);
}

int request_handler::writer(int redir_mode) {
// PAR DEFAULT ON CONSIDÈRE QUE TOUT SE PASSE BIEN ON CHANGE PAR LA SUITE LE STATUS SI UNE EXCEPTION ARRIVE
/* PROBLEM REDONDANT AVEC LA METHODE GET -> VA FALLOIR CHOISIR*/
	add_all_field(); 
	if (redir_mode == HEAD)
		redir_mode = NONE;
	else
		redir_mode = add_body();
	_hrx.clear();
	_htx.clear();
	return (redir_mode);
}

int	get_dec_len(std::string msg)	{
	int		size;
	size_t	pos;
	std::string	buf;

	buf.clear();
	pos = 0;
	while (msg[pos] && isdigit(msg[pos]))
	{
		buf += msg[pos];
		pos++;
	}
	if (buf.empty())
		return (-1);
	else
		size = std::strtoul(buf.c_str(), NULL, 10);
	return (size);
}

int	request_handler::cgi_writer()
{
	int	status;
	std::stringstream ss;

	status = -1;
	if (!_htx["Status"].empty())
	{
		status = get_dec_len(_htx["Status"][0].substr(_htx["Status"][0].find(": ") + 2));
		_htx["Status"].clear();
	}
		
	if (status != -1)
		gen_startLine( status );
	else
		gen_startLine( 200 );
	if (!_body.empty())
		gen_CLength();
	add_all_field(); 
#ifdef _debug
	std::cout << "All fields added" << std::endl;
#endif
	_response += _body;
#ifdef _debug
	std::cout << "Body added" << std::endl;
#endif
	_path.clear();
	_body.clear();
	_hrx.clear();
	_htx.clear();
	return (NONE);
}

void	request_handler::gen_allowed()	{
#ifdef _debug
	std::cout << "Generating Allow header for response" << std::endl;
#endif
	if (_htx["Allowed"].empty())
		_htx["Allowed"] = std::vector<std::string>();
	_htx["Allowed"].push_back("Allow: ");
	for (std::vector<std::string>::iterator it = _si[_s_id].location[_l_id].allowed_method.begin(); it != _si[_s_id].location[_l_id].allowed_method.end(); it++)
	{
		_htx["Allowed"].push_back(*it);
		if (it + 1 != _si[_s_id].location[_l_id].allowed_method.end())
		_htx["Allowed"].push_back(", ");
	}
	_htx["Allowed"].push_back("\r\n");
}

	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */

void	request_handler::gen_startLine(size_t ret_code)
{
	std::stringstream	ss;

	_htx["A"].clear();
	_htx["A"].push_back("HTTP/1.1 "); // version (static)
	ss << ret_code;
	_htx["A"].push_back(ss.str()); // status code (dynamic) -> 200
	_htx["A"].push_back(ret_string(ret_code)); // status msg (dynamic) -> OK
	_htx["A"].push_back("\r\n");
// MODIFIE LE PNG DS ERROR_PAGE.HTML SI NECESSAIRE
	if (ret_code >= 300) {
		fstream error_file("files/error_pages/4xx.html");
		error_file.seekg(1252);
		error_file.write(_htx["A"][1].c_str(), 3);
		error_file.close();
	}
}

void	request_handler::gen_date()
{
	std::string	date = "Date: ";
	time_t		timer;
	struct tm	*info;
	char		timestamp[36];

	timer = time(NULL);
	info = localtime(&timer);
	strftime(timestamp, 36, "%a, %d %h %Y %H:%M:%S GMT", info);
	date.append(timestamp);
	date.append("\r\n");

	if (_htx["Date"].empty())
		_htx["Date"] = std::vector<std::string>();
	_htx["Date"].push_back(date);
}

void	request_handler::gen_serv() /* PROBLEM : S'IL N'Y A PAS DE CHAMP Server DS LA REQUETE TU SEGV PAR EX POUR UN POST DS GD TAILLE*/
{
	if (_htx["Server"].size() != 3)
		_htx["Server"].resize(3, string());
	_htx["Server"][0] = "Server: ";// HEADER_LABEL
	if (!_hrx["Host:"].empty())
		_htx["Server"][1] = _hrx["Host:"][0];// IP & HOST
	else
		_htx["Server"][1] = "127.0.0.1";// IP & HOST
	_htx["Server"][2] = "\r\n";
}

// génération du field Content-Type
void	request_handler::gen_CType(string ext) /* PROBLEM : mieux vaudrait extraire ça d'un fichier et le récup ici (serait plus élégant)*/
{
// Capture file.ext(ension)
	size_t	pos;

	if (ext.empty())
	{
		pos = _hrx["A"][1].find_last_of(".") + 1;
		if (pos != std::string::npos)
			ext = _hrx["A"][1].substr(pos);
	}
	else if (_path == "/" && (pos = _si[_s_id].location[_l_id].root.find_last_of(".") + 1) != std::string::npos)
		ext = _si[_s_id].location[_l_id].root.substr(pos);
	else if (is_folder(_path))
	{
		ext = "html";
	}
#ifdef _debug_
std::cout << "file ext asked : " << ext << endl;
#endif
	std::string type = mime_string(ext.c_str());
#ifdef _debug_
	std::cout << "MIME type is : " << type << std::endl;
#endif
	_htx["Content-Type"].clear();
	_htx["Content-Type"].push_back("Content-Type: ");
	_htx["Content-Type"].push_back(type);
	_htx["Content-Type"].push_back("\r\n");
}

// génération du field Content-Length et d'un status d'erreur (413) si length > max_file_size
void	request_handler::gen_CLength()	{
	_htx["Content-Length"].clear();
	_htx["Content-Length"].push_back("Content-Length: "); // HEADER_LABEL

	stringstream ss;
	if (_body.empty()) {
		ifstream fs(_path.c_str(), std::ifstream::binary | std::ifstream::ate);
		ss << fs.tellg();
		fs.close();
	}
	else
		ss << _body.size();
	_htx["Content-Length"].push_back( ss.str());
	_htx["Content-Length"].push_back( "\r\n"  );
// PLUS DE REQUETE TROP LONGUE POUR LA REPONSE AU CLIENT (A DECOMMENTER -AV SON HOMOLOGUE DS gen_CLength()- SI CHANGEMENT D'AVIS)
}

	/* FUNCTION SECONDAIRE : UTILITAIRES */

// reconnait quel server_virtuel va traiter la requete et initialise _s_id
void request_handler::set_server_id(void)
{
	size_t colon_pos = _hrx["Host:"][0].find_first_of(":");
	string host(_hrx["Host:"][0].substr(0,colon_pos));
	string port(_hrx["Host:"][0].substr(colon_pos +1));
	for (size_t i = 0; i < _si.size(); ++i)
		if ( (_si[i].host == host || _si[i].server_name == host) && _si[i].port == port ) {
			_s_id = i;
#ifdef _debug_
			std::cout << RED "_s_id : " RESET << _s_id << endl;
#endif
			return ;
		}
// SI LE HOST:PORT N'A PAS ETE TROUVE ON SELECT LE PREMIER SERVER CORRESPONDANT
	for (size_t i = 0; i < _si.size(); ++i)
		if (_si[i].port == port) {
			_s_id = i;
			break;
		}
#ifdef _debug_
		std::cout << "host : " << host << ", port : " << port << ", id : " << _s_id << endl;
#endif
}

/* ◦ Le premier serveur pour un host :port sera le serveur par défaut pour cet
host :port (ce qui signifie qu’il répondra à toutes les requêtes qui n’appar-
tiennent pas à un autre serveur) */



void request_handler::handle_get_rqst(void)
{
}

int	request_handler::create_file(std::string& path)	{
	std::cout << "Creating file: " << path << std::endl;
	redir_fd[1] = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
	if (redir_fd[1] == -1)
	{
		if (redir_fd[0] != -1)
		{
			close(redir_fd[0]);
			redir_fd[0] = -1;
		}
		redir_fd[1] = -1;
		return (NONE);
	}
	else
		return (WRITE);
}

int request_handler::multipart_form(string& boundary, string& msg)	{
	size_t	pos;
	string	tmp, buf, path;
	ofstream	out;

#ifdef _debug_
	std::cout << "Parsing multipart Form" << std::endl;
#endif
	pos = msg.find(boundary);
	if (pos != string::npos)
	{
		msg = msg.substr(pos + boundary.length());
		if (msg.substr(0, 2) == "\r\n")
			msg = msg.substr(2);
		else if (msg.substr(0, 2) == "--")
		{
#ifdef _debug_
			std::cout << "Boundary end found" << std::endl;
#endif
			msg = msg.substr(2);
			return (NONE);
		}
		while ((pos = msg.find("\r\n")) != string::npos)
		{
			if (pos == 0)
				break ;
			buf = msg.substr(0, pos + 2);
			msg = msg.substr(pos + 2);
			if (buf == "\r\n")
				break ;
			else
				tmp.append(buf);
		}
		if ((pos = tmp.find("filename=\"")) != string::npos)
		{
#ifdef _debug_
			std::cout << "Extracting multipart/form filename" << std::endl;
#endif
			buf = tmp.substr(pos + strlen("filename=\""), tmp.find("\r\n"));
			path = buf.substr(0, buf.find("\""));
#ifdef _debug_
			std::cout << "Path set to : " << path << std::endl;
#endif
		}
		if (msg.substr(0, 2) == "\r\n")
			msg = msg.substr(2);
		buf.clear();
		if ((pos = msg.find(boundary + "--")) != string::npos)
		{
#ifdef _debug_
			std::cout << "Found end of multipart body" << std::endl;
#endif
			buf.append(msg.substr(0, pos));
#ifdef _debug_
			std::cout << "searching for trailing character at buffer end: " << (int)buf[buf.length() -2] << " ,"
				<< (int)buf[buf.length() -1] << std::endl;
#endif
			if (buf.substr(buf.length() - 2, 2) == "\r\n")
				buf = buf.substr(0, buf.length() - 2);
			_body = buf;
			msg = msg.substr(pos + (boundary + "--").length());
			if (msg.substr(0, 2) == "\r\n")
				msg = msg.substr(2);
			_path += ("/" + path);
			return (create_file(_path));
		}
	}
	return (NONE);
}

int request_handler::handle_put_rqst(void) 
{
	int			redir_mode;
	std::string	tmp;
	std::string	boundary;

	redir_mode = NONE;
#ifdef _debug_
	std::cout << "Handling PUT request" << std::endl;
#endif

	if (_hrx["Content-Length:"].empty() && _hrx["Transfer-Encoding:"].empty())
	{
		gen_startLine( 400 );
		return (NONE);
	}

// POUR LIMITER LA TAILLE DU BODY DU CLIENT => JE NE SAIT PAS ENCORE COMMENT GET LA LOCATION CONCERNÉE
	if (_hrx["Content-Type:"].size() > 1)
		boundary = "--" + _hrx["Content-Type:"][1].substr(strlen("boundary="));
	// En cas de body plus long qu'autorisé -> 413 (Request Entity Too Large) 
	if (!_si[_s_id].max_file_size.empty() && _hrx["BODY"][0].size() > static_cast<size_t>(atoi(_si[_s_id].max_file_size.c_str())) ) {
#ifdef _debug_
		std::cout << "Max file size was set" << std::endl;
#endif
		gen_startLine( 413 ); 
		return (NONE);
	}
	else
	{
		if (!_hrx["BODY"].empty())
		{
			if (!boundary.empty() || boundary == "--")
			{
				_response.clear();
				redir_mode = multipart_form(boundary, _hrx["BODY"][0]);
				if (redir_mode != NONE)
					return (redir_mode);
				resolve_path();
			}
			else
			{
				redir_mode = create_file(_path);
				if (redir_mode != NONE)
				{
					_body = _hrx["BODY"][0];
					return (redir_mode);
				}
				resolve_path();
			}
		}
	}
	return (redir_mode);
}

int request_handler::handle_post_rqst(void) 
{
	int			redir_mode;
	std::string	tmp;
	std::string	boundary;

	redir_mode = NONE;
#ifdef _debug_
	std::cout << "Handling POST request" << std::endl;
#endif

	if (_hrx["Content-Length:"].empty() && _hrx["Transfer-Encoding:"].empty())
	{
		gen_startLine( 400 ); 
		return (NONE);
	}

// POUR LIMITER LA TAILLE DU BODY DU CLIENT => JE NE SAIT PAS ENCORE COMMENT GET LA LOCATION CONCERNÉE
	if (_hrx["Content-Type:"].size() > 1)
		boundary = "--" + _hrx["Content-Type:"][1].substr(strlen("boundary="));

	// En cas de body plus long qu'autorisé -> 413 (Request Entity Too Large) 
	if (!_si[_s_id].max_file_size.empty() && _hrx["BODY"][0].size() > static_cast<size_t>(atoi(_si[_s_id].max_file_size.c_str())) ) {
		gen_startLine( 413 ); 
		return (NONE);
	}
	else
	{
		if (!_hrx["BODY"].empty())
		{
			if (!boundary.empty())
			{
				_response.clear();
				redir_mode = multipart_form(boundary, _hrx["BODY"][0]);
				if (redir_mode != NONE)
					return (redir_mode);
				resolve_path();
			}
			else
			{
				redir_mode = create_file(_path);
				if (redir_mode != NONE)
				{
					_body = _hrx["BODY"][0];
					return (redir_mode);
				}
				resolve_path();
			}
		}
	}
	return (redir_mode);
}

bool	request_handler::is_folder(std::string path)	{
	struct stat sb;

	if (lstat(path.c_str(), &sb) == -1)
		perror("lstat");
	if ((sb.st_mode & S_IFMT) == S_IFDIR)	{
#ifdef _debug_
		std::cout << "directory" << std::endl;
#endif
		return (true);
	}
	return (false);
}

bool	request_handler::is_regular_file(std::string path)	{
	struct stat sb;

	if (lstat(path.c_str(), &sb) == -1)
		perror("lstat");
	if ((sb.st_mode & S_IFMT) == S_IFREG)	{
#ifdef _debug_
		std::cout << "regular file" << std::endl;
#endif
		return (true);
	}
	return (false);
}

// Permet de séléctionner la location qui partage le plus avec l'url comme le fait nginx,
// si l'url a plusieurs niveau de dossiers, choisi la loc la plus adaptée
// identifie l'index de la location correspondante à l'url spcécifiée (_l_id)
int	request_handler::resolve_path()
{
	bool	redirect;
	size_t	len;

	redirect = 0;
#ifdef _debug_
	std::cout << GREEN "DS RESOLVE_PATH [" + _hrx["A"][1] + "], _s_id : " << _s_id <<  " _path (cleared afterward): " << _path << endl;
#endif
	_path.clear();
// REMOVE MULTIPLE '/' AND THE '/' AT URL'S END
	clean_url(_hrx["A"][1]);
	if (_hrx["A"][1].find("/error_pages/", 0,13) == 0) {
		_path = (_hrx["A"][1][0] == '/' ? "files" + _hrx["A"][1] : "files/" + _hrx["A"][1]);
		return true;
	}

	_l_id = 0;
	int index = _si[_s_id].location.size() - 1;
	len = 0;
	for (vector<locati_info>::reverse_iterator it = _si[_s_id].location.rbegin(); it != _si[_s_id].location.rend(); ++it, --index)
	{
		if ((_hrx["A"][1].find( it->location.c_str(), 0 , it->location.size()) == 0 && it->location.size() > len) || (!len && !index)) // || si on a rien trouvé la location[0] est le default
		{
			// S'IL Y A UNE DIRECTIVE_RETURN À L'INTERIEURE DE LA LOCATION
			if (!it->retour.empty()) {
				for (vector<locati_info>::iterator it2 = _si[_s_id].location.begin(); it2 != _si[_s_id].location.end(); ++it2)
					if (it2->location == it->retour.back()) {
						string::const_iterator c_it = it->retour[0].begin();
						while (c_it != it->retour[0].end() && std::isdigit(*c_it))
							++c_it;
						if ( !it->retour[0].empty() && c_it == it->retour[0].end())
						{
							stringstream	ss(it->retour[0]);
							size_t			ret;

							ss >> ret;
							gen_startLine( ret );
						}

						_path = it2->root + "/";
						_l_id = it2 - _si[_s_id].location.begin();
						redirect = 1;
						break ;
					}
			}
			else {
			 	_path = it->root + "/";
				_l_id = index;
				redirect = 0;
			}
		//	 SI POST ET PRESENCE DIRECTIVE_DOWNLOAD À L'INTERIEURE DE LA LOCATION
			if ((is_cgi(_hrx["A"], _si[_s_id].location[_l_id].cgi_file_types) == -1) && !_hrx["BODY"].empty() && (_hrx["A"][0] == "POST" || _hrx["A"][0] == "PUT") && !it->upload_path.empty())	{
#ifdef _debug_
				std::cout << RED "UPLOAD_PATH : " << it->upload_path << endl;
	#endif
				_path = it->upload_path + '/'; // on prend le path_ de download_path tel quel (pas de combinaison av root mettre += si on veut le combiner)
				if (_hrx["Content-Type:"].empty() || (!_hrx["Content-Type:"].empty() && _hrx["Content-Type:"][1].find("boundary") == std::string::npos))
					_path += _hrx["A"][1].substr(it->location.size());
			}
			else
				_path += _hrx["A"][1].substr(it->location.size());
			len = it->location.length();
		}
	}
#ifdef _debug_
	std::cout << BLUE "path : " RESET << _path << endl;
#endif
// AFFACER LES '/' EN PRÉFIXE (POUR OUVRIR DEPUIS LA RACINE DE NOTRE DOSSIER ET PAS DEPUIS LA RACINE MERE)
	while (_path[0] == '/')
		_path.erase(_path.begin());
	clean_url(_path);
#ifdef _debug_
	std::cout << BLUE "resolved_path : " RESET << _path << endl;
	std::cout << "location [" << _l_id << "] : " + _si[_s_id].location[_l_id].location << endl;
#endif
	if (redirect == 1)
	{
		gen_startLine( 303 );
		_htx["Location"].clear();
		_htx["Location"] = std::vector<std::string>();
		_htx["Location"].push_back("Location: ");
		reverse_resolve_path(_path);
		_htx["Location"].push_back(_path);
		_htx["Location"].push_back("\r\n");
		return (true);
	}

// VERIFIE SI LA MÉTHODE DS LA LOCATION CONCERNÉE EST AUTORISÉE
	if ((ext_id = is_cgi(_hrx["A"], _si[_s_id].location[_l_id].cgi_file_types) != -1)
		&& !_si[_s_id].location[_l_id].cgi_path.empty())
		return (false);
	else
		return !is_method_allowed();
}

// Verifie si elle est autorisee ds le cas d'une methode inconnue faite av curl -X (maj gen_stratLine 405 si besoin)
// puis si la méthode ds la location concernée est autorisée ou non (maj gen_stratLine 403 si besoin)
bool request_handler::is_method_allowed(void)
{/* PROBLEME (A TESTER) */
#ifdef _debug_
	std::cout << MAGENTA "is_method_allowed : " RESET;
	#endif
	bool allowed = false;
	const char *array[] = {"GET", "PUT", "HEAD", "POST", "DELETE", "PATCH", NULL};

	for (const char**strs = array; *strs; ++strs){
		if (*strs == _hrx["A"][0])
			allowed = true;
	}
	if (!allowed){
		gen_startLine( 400 );
#ifdef _debug_
		std::cout << MAGENTA << "400 Bad Request" << RESET << endl;
	#endif
		return allowed;
	}
	allowed = false;
	for (size_t i = 0; i < _si[_s_id].location[_l_id].allowed_method.size(); ++i)
		if (_si[_s_id].location[_l_id].allowed_method[i] == _hrx["A"][0])
			allowed = true;
	if (!allowed)
		gen_startLine( 405 );
#ifdef _debug_
	std::cout << MAGENTA << (allowed ? "oui\n" : "non\n") << RESET;
	#endif
	return allowed;
}

// Détecte si c'est un fichier normal ou s'il n'existe pas (maj de la statut-line si besoin)
// Si c'est un dossier check l'autoindex ou la presence d'une directive index et affiche le contenu du dossier ou ajoute le fichier index respectivement
// Si erreur lors de l'ouverture du fichier renvoie le _path sur les pages d'erreurs
int request_handler::file_type()
{
#ifdef _debug_
	std::cout << GREEN "DS FILE_TYPE()" RESET <<  " _path : " << _path << endl;
	#endif
	struct stat sb;

	bzero(&sb, sizeof(sb));
	if (lstat(_path.c_str(), &sb) == -1)
		perror("lstat");

	switch (sb.st_mode & S_IFMT) {
		case S_IFDIR:
#ifdef _debug_
		std::cout << "directory" << std::endl;
#endif
			if (_si[_s_id].location[_l_id].autoindex == "on") {
// PROBLEM
				generate_folder_list();
				return 1;
			}
			if (!_si[_s_id].location[_l_id].index.empty()) {
				_path += _path[_path.size() -1] == '/' ? _si[_s_id].location[_l_id].index : '/' + _si[_s_id].location[_l_id].index;		
				file_type();									
			}
			else
				gen_startLine( 403 );
			break;
		case S_IFREG:
#ifdef _debug_
		std::cout << "regular file" << std::endl;
		#endif
			break;
		default:
			if (atoi(_htx["A"][1].c_str()) < 400)
				gen_startLine( 404 );
#ifdef _debug_
			std::cout << RED "unknown path : " RESET << _path.c_str() << std::endl;
	#endif
			break;
	}
// AIGUILLE LE PATH SUR LA PAGE D'ERREUR CORRESPONDANTE
	if (atoi(_htx["A"][1].c_str()) >= 400)
		_path = _si[_s_id].error_page.empty() || _si[_s_id].error_page.find("files/error_pages") != string::npos ? "files/error_pages/4xx.html" : _si[_s_id].error_page + _htx["A"][1] + ".html";
#ifdef _debug_
	std::cout << "_path : " << _path.c_str() << std::endl;
#endif
	return 0;
}

// Créé la liste de fichier(s) à afficher ds _body
void request_handler::generate_folder_list()
{
	DIR *dpdf;
	set<string> st;
	struct dirent *epdf;

	dpdf = opendir(_path.c_str());
	if (dpdf != NULL)
	   	while ((epdf = readdir(dpdf))) {
			st.insert(epdf->d_name);
#ifdef _debug_
	    	std::cout << epdf->d_name << std::endl;
#endif
		}
	closedir(dpdf);

	fstream autoindex_file("config_files/autoindex.html");
	if (!autoindex_file.is_open())
		throw (std::runtime_error("Couldn't open : configuration_files/autoindex.html"));
// BUFFERISE LE TEMPLATE HTML POUR Y AJOUTER LE CONTENU DU DOSSIER
	std::stringstream buffer;
	buffer << autoindex_file.rdbuf();
	_body = buffer.str();
	autoindex_file.close();
	for (set<string>::iterator i = st.begin(); i != st.end(); ++i)
		_body.append("<div style='padding:10px;'><a href=\"" + _hrx["A"][1] + '/' + *i + "\">" + *i + "</a></div>");
	_body.append("</div></body></html>");
}

// Clear la string (response) et y ajoute tous les field, puis clear _hrx
void request_handler::add_all_field()
{
	_response.clear();
	for (std::map<string, vector<string> >::iterator it = _htx.begin(); it != _htx.end(); it++)
		for (size_t i = 0, j = it->second.size(); i < j; ++i)
			_response += it->second[i];
	_response += "\r\n";

	std::cout << BLUE "Response Headers (add_all_field()) :\n" RESET << _response << endl;
}

// Ajout du fichier ou du body À LA SUITE des header dans response
int request_handler::add_body()
{
	if (!_body.empty()) {
		_response += _body;
		_body.clear();
#ifdef _debug_
		std::cout << "body appended" << std::endl;
#endif
		return (NONE);
	}
// S'IL S'AGIT D'UN GET OU D'UN POST ON JOINS LE FICHIER
	if ((!_hrx["A"].empty() && (_hrx["A"][0] == "GET" || _hrx["A"][0] == "POST")) || (!_htx["A"].empty() && _htx["A"][1] == "303") || _response.substr(0, _response.find("\r\n\r\n")).find("303") != std::string::npos) {
		redir_fd[0] = open(_path.c_str(), O_RDONLY | O_NONBLOCK);
		if (redir_fd[0] == -1)
		{
			redir_fd[0] = -1;
			redir_fd[1] = -1;
#ifdef _debug_
			std::cout << "Open error" << std::endl;
#endif
			return (NONE);
		}
#ifdef _debug_full_
		std::cout << _body << std::endl;
#endif
		return (READ);
	}
	return (NONE);
}

// Remove multiple '/' and the '/' at url's end
void request_handler::clean_url(string& str)
{
	for (size_t i = 0; i < str.size(); ++i)
		while (str[i] == '/' && (str[i + 1] == '/' || str[i + 1] == '\0') && str.size() > 1)
			str.erase(i, 1);
}


	/* FUNCTION GETTER / SETTER */

std::string request_handler::get_path(void) {return _path;}

std::string request_handler::get_response(void) {return _response;}

std::string request_handler::get_body(void) {return _body;}

void request_handler::set_body(const string& str)	{
	_body = str;
}

void	request_handler::fill_redir_fd(int (*loc_fd)[2], pid_t *cgi_pid) {
	if (redir_fd[0] != -1)
		(*loc_fd)[0] = redir_fd[0];
	if (redir_fd[1] != -1)
		(*loc_fd)[1] = redir_fd[1];
	redir_fd[0] = -1;
	redir_fd[1] = -1;
	if (redir_pid != -1)
		*cgi_pid = redir_pid;
	redir_pid = -1;
}

	/* FUNCTION DE DEBUG */

void request_handler::display(void) {
std::cout << GREEN ITALIC UNDERLINE "DISPLAY HEADER INFORMATION" RESET GREEN " :" RESET << endl;
	for (map<string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it) {
	std::cout << it->first << " ";
		for (size_t i = 0; i < it->second.size(); ++i)
		std::cout << it->second[i] << ", ";
		std::cout << endl;
	}
}

std::vector<std::string> request_handler::extract_env(std::map<std::string, std::vector<std::string> >& mp,const server_info& _s)
{
	std::vector<std::string>	env;
	std::string			tmp;
	std::string			buf(mp["A"][1]);
	std::string			var;

	tmp =  "REQUEST_METHOD=";
	tmp += _hrx["A"][0];
	env.push_back(tmp);
	tmp =  "REDIRECT_STATUS=200";
	env.push_back(tmp);
	tmp = "SERVER_SOFTWARE=";
	tmp += "HTTP/1.1";
	env.push_back(tmp);
	tmp = "SERVER_NAME=";
	tmp += _s.server_name;
	env.push_back(tmp);
	tmp = "GATEWAY_INTERFACE=CGI/1.1";
	env.push_back(tmp);
	tmp = "SERVER_PROTOCOL=HTTP/1.1";
	env.push_back(tmp);
	tmp = "SERVER_PORT=";
	tmp += _s.port;
	env.push_back(tmp);
	tmp = "REQUEST_METHOD=";
	tmp += mp["A"][0];
	env.push_back(tmp);
	tmp = "PATH_INFO=";
	if (!mp["Path-Info"].empty())
	{
		for (size_t j = 0; j < mp["Path-Info"].size(); ++j)
			tmp+= mp["Path-Info"][j];
	}
	env.push_back(tmp);
	tmp = "PATH_TRANSLATED=";
	if (!mp["Path-Translated"].empty())
	{
		for (size_t j = 0; j < mp["Path-Translated"].size(); ++j)
			tmp+= mp["Path-Translated"][j];
	}
	env.push_back(tmp);
	tmp.clear();
	tmp = "SCRIPT_NAME=";
	if (!mp["Script-Name"].empty())
	{
		for (size_t j = 0; j < mp["Script-Name"].size(); ++j)
			tmp+= mp["Script-Name"][j];
	}
	env.push_back(tmp);
	tmp = "SCRIPT_FILENAME=";
	if (!mp["Script-Name"].empty())
	{
		for (size_t j = 0; j < mp["Script-Filename"].size(); ++j)
			tmp+= mp["Script-Filename"][j];
	}
	env.push_back(tmp);
	tmp = "REQUEST_URI=";
	if (!mp["Path-Info"].empty())
	{
		for (size_t j = 0; j < mp["Path-Info"].size(); ++j)
			tmp+= mp["Path-Info"][j];
	}
	env.push_back(tmp);
	tmp = "QUERY_STRING=";
	if (!mp["Query-String"].empty())
	{
		for (size_t j = 0; j < mp["Query-String"].size(); ++j)
			tmp+= mp["Query-String"][j];
	}
	env.push_back(tmp);
	tmp = "REMOTE_HOST=";
	if (!mp["Host:"].empty())
	{
		for (size_t j = 0; j < mp["Host:"].size(); ++j)
			tmp+= mp["Host:"][j];
	}
	env.push_back(tmp);
	tmp = "DOCUMENT_ROOT=";
	if (!mp["Document-Root"].empty())
	{
		for (size_t j = 0; j < mp["Document-Root"].size(); ++j)
			tmp+= mp["Document-Root"][j];
	}
	env.push_back(tmp);
	tmp = "HTTP_HOST=";
	tmp += _si[_s_id].host;
	tmp += ":" + _si[_s_id].port;
	env.push_back(tmp);
	tmp = "CONTENT_TYPE=";
	if (!mp["Content-Type:"].empty())
	{
		for (size_t j = 0; j < mp["Content-Type:"].size(); ++j)
			tmp+= mp["Content-Type:"][j];
	}
	env.push_back(tmp);
	tmp = "CONTENT_LENGTH=";
	if (!mp["Content-Length:"].empty())
	{
		for (size_t j = 0; j < mp["Content-Length:"].size(); ++j)
			tmp+= mp["Content-Length:"][j];
	}
	else
		tmp+="0";
	env.push_back(tmp);
	tmp = "HTTP_USER_AGENT=";
	if (!mp["User-Agent:"].empty())
	{
		for (size_t j = 0; j < mp["User-Agent:"].size(); ++j)
			tmp+= mp["User-Agent:"][j];
	}
	env.push_back(tmp);
	tmp = "HTTP_REFERER=";
	if (!mp["Referer"].empty())
	{
		for (size_t j = 0; j < mp["Referer"].size(); ++j)
			tmp+= mp["Referer"][j];
	}
	env.push_back(tmp);
	tmp = "TMPDIR=";
	if (!_si[_s_id].location[_l_id].upload_path.empty())
		tmp += _si[_s_id].location[_l_id].upload_path + '/';
	else if (!_si[_s_id].location[_l_id].root.empty())
		tmp += _si[_s_id].location[_l_id].root;
	env.push_back(tmp);
	return (env);
}

void	request_handler::clean_body()
{
	std::string	tmp;
	std::string	index;
	size_t	pos;
	
#ifdef _debug_full_
	std::cout << "Printing cgi return body: " << std::endl;
#endif
	while ((pos = _body.find("\r\n")) != std::string::npos)
	{
		tmp = _body.substr(0, pos + 2);
		_body = _body.substr(pos + 2);
		if (tmp == "\r\n")
			break ;
		if ((pos = tmp.find(":")) != std::string::npos)
		{
			index = tmp.substr(0, pos);
			if (index != "X-Powered-By")
				_htx[index].push_back(tmp.substr(0, tmp.find("\r\n") + 2));
		}
	}
}

void	request_handler::cgi_var_init()	{
	size_t				len, pos;
	std::string			var;
	char				str[100];

	bzero(&str, sizeof(str));
	//CGI variables initialisation
	if ((len = _path.find("?")) != std::string::npos)
	{
		var = _path.substr(len + 1);
		_path = _path.substr(0, len);
	}
#ifdef _debug_
	std::cout << "PATH=" << _path << std::endl;
#endif
	_hrx.insert(std::make_pair("Path-Translated", std::vector<std::string>()));
	var = _path;
	_hrx["Path-Translated"].push_back(var);
	var.clear();
	_hrx.insert(std::make_pair("Script-Filename", std::vector<std::string>()));
	getcwd(str, sizeof(str));
	var = str;
	var += "/";
	var += _path;
	_hrx["Script-Filename"].push_back(var);
	_hrx.insert(std::make_pair("Script-Name", std::vector<std::string>()));
	var = var.substr(var.find_last_of("/") + 1);
	_hrx["Script-Name"].push_back(var);
	_hrx.insert(std::make_pair("Document-Root", std::vector<std::string>()));
	pos = _path.find_last_of("/");
	var = _path.substr(0, pos);
	if (var.substr(0, 2) == "./")
		var = var.substr(2);
	_hrx["Document-Root"].push_back(var);
	_hrx.insert(std::make_pair("Path-Info", std::vector<std::string>()));
	var = _hrx["A"][1].substr(0, _hrx["A"][1].find("?"));
	_hrx["Path-Info"].push_back(var);
	var.clear();
	_hrx.insert(std::make_pair("Query-String", std::vector<std::string>()));
	if (_hrx["A"][1].find("?") != std::string::npos)
		var = _hrx["A"][1].substr(_hrx["A"][1].find("?") + 1);
	_hrx["Query-String"].push_back(var);
	var.clear();
}

int	request_handler::handle_cgi(void)
{
	std::vector<std::string>	env;

	if (_s_id == -1)
	{
#ifdef _debug_
		std::cout << "Invalid server id: " << _s_id << std::endl;
#endif
		return (NONE);
	}
	else
	{
		if (_si[_s_id].location[_l_id].cgi_path.empty())
		{
			gen_startLine( 403 );
			return (NONE);
		}
#ifdef _debug_
		std::cout << "Launching cgi" << std::endl;
#endif
		cgi_var_init();
		env = extract_env(_hrx, _si[_s_id]);
		if ((redir_pid = go_cgi(&redir_fd, _si[_s_id].location[_l_id].cgi_path, env)) == -1 || (redir_fd[0] == -1 || redir_fd[1] == -1))
		{
			if (redir_fd[0] != -1)
			{
				close(redir_fd[0]);
				redir_fd[0] = -1;
			}
			if (redir_fd[1] != -1)
			{
				close(redir_fd[1]);
				redir_fd[1] = -1;
			}
			redir_pid = -1;
			return (NONE);
		}
		_body = _hrx["BODY"][0];
#ifdef _debug_
		std::cout << "Buffer copied in body" << std::endl;
#endif
		return (CGI_IN);
	}
	return (NONE);
}

void	request_handler::clean(void)	{
	_hrx.clear();
	_htx.clear();
	_path.clear();
	_body.clear();
	_response.clear();
}

void	loc_len(std::vector<locati_info>& locs, std::string& path)	{
	std::string*	longest_loc;
	std::string*	tmp;

	longest_loc = NULL;
	tmp = NULL;
	for (std::vector<locati_info>::iterator it = locs.begin(); it != locs.end(); it++)
	{
		if (path.find(it->root) == 0)
		{
			if (longest_loc == NULL)
			{
				longest_loc = &it->root;
				tmp = &it->location;
			}
			else if (it->root.length() > longest_loc->length())
			{
				longest_loc = &it->root;
				tmp = &it->location;
			}
		}
	}
	if (longest_loc != NULL)
	{
		path = path.substr(longest_loc->length());
		path = *tmp + path;
	}
}

std::string	request_handler::reverse_resolve_path(std::string &loc_path)	{
	std::string	ret;

	(void)loc_path;
	ret = "http://";
	ret += _si[_s_id].host;
	ret += ":";
	ret += _si[_s_id].port;
	loc_len(_si[_s_id].location, loc_path);
	clean_url(loc_path);
	ret += loc_path;
	ret += "\r\n";
	return (ret);
}

int	request_handler::create_write_resp(std::string &file_path)	{
	int	redir_mode;

	gen_startLine( 303 );


	gen_date();
	gen_serv();
	gen_CLength();
	gen_CType(file_path.substr(file_path.find_last_of(".") + 1));

	_htx["Server"][1] += ":" + _si[_s_id].port;
	if (_htx["Location"].empty())
		_htx["Location"] = std::vector<std::string>();
	_htx["Location"].push_back("Location: ");
	_htx["Location"].push_back(reverse_resolve_path(file_path));
	add_all_field(); 
#ifdef _debug_
	std::cout << "Searching for file at address: " << _path << std::endl;
	std::cout << "Sent with virtual address: " << file_path << std::endl;
#endif
	_body.clear();
	redir_mode = add_body();


	if (redir_mode == NONE)
	{
#ifdef _debug_
		std::cout << "File not found" << std::endl;
#endif
		if (redir_fd[0] == -1)
		{
			close(redir_fd[0]);
			redir_fd[0] = -1;
		}
		if (redir_fd[1] == -1)
		{
			close(redir_fd[1]);
			redir_fd[1] = -1;
		}
	}
	_hrx.clear();
	_htx.clear();
	return (redir_mode);
}

int	get_hex_len(std::string& msg)	{
	int		size;
	size_t	pos;
	std::string	buf;

	buf.clear();
	pos = 0;
	while (msg[pos] && isxdigit(msg[pos]))
	{
		buf += msg[pos];
		pos++;
	}
	if (buf.empty())
		return (-1);
	else
		size = std::strtoul(buf.c_str(), NULL, 16);
	return (size);
}

std::string	request_handler::clean_chunk(std::string& buf)
{
	size_t	
	size_t	pos;
	int	chunk_mode;
	int	size;
	std::string	ret;

	chunk_mode = CHUNK_PARSING_REQ;

#ifdef _debug_
	std::cout << "Clearing buffer" << std::endl;
#endif
	if (buf.empty())
		return (buf);
	while (chunk_mode != CHUNK_COMPLETE
			&& chunk_mode != BAD_REQUEST)
	{
		size = get_hex_len(buf);
		if (buf.substr(0, 5) == "0\r\n\r\n")
		{
			chunk_mode = CHUNK_COMPLETE;
			break ;
		}
		if (size == -1)
		{
#ifdef _debug_
			std::cout << "Bad Request !" << std::endl;
#endif
			chunk_mode = BAD_REQUEST;
			return (std::string());
		}
		else if (size == 0)
		{
			if (buf.substr(0, 4) == "\r\n\r\n")
				chunk_mode = CHUNK_COMPLETE;
			else if (buf.length() == 0)
				chunk_mode = CHUNK_COMPLETE;
		}
		if ((pos = buf.find("\r\n")) == std::string::npos)
		{
#ifdef _debug_
			std::cout << "Bad Request !" << std::endl;
#endif
			chunk_mode = BAD_REQUEST;
			return (std::string());
		}
		buf = buf.substr(pos + 2);
		if (buf.length() <= (size_t)size)
		{
#ifdef _debug_
			std::cout << "Bad Request !" << std::endl;
#endif
			chunk_mode = BAD_REQUEST;
			return (std::string());
		}
		ret.append(buf.substr(0, size));
		buf = buf.substr(size);
#ifdef _debug_
		std::cout << "Size remaining : " << buf.length() << std::endl;
#endif

		if (buf.substr(0, 2) != "\r\n")
		{
#ifdef _debug_
			std::cout << "Bad Request !" << std::endl;
#endif
			chunk_mode = BAD_REQUEST;
			return (std::string());
		}
		buf = buf.substr(2);
	}
	if (chunk_mode == CHUNK_COMPLETE)
		chunk_mode = TRANSMISSION_OVER;
	buf.clear();
	return (ret);
}

void	create_tmp_file(int	*fd)	{
	std::string path = "./files/tmp/cgi_buffer";

#ifdef _debug_
	std::cout << "Creating tmp file : " << path << std::endl;
#endif
	*fd = open(path.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);
	fcntl(*fd, F_SETFL, O_NONBLOCK);
}

int	request_handler::handle_cgi_fd(void)
{
	std::vector<std::string>	env;

	if (_s_id == -1)
	{
#ifdef _debug_
		std::cout << "Invalid server id: " << _s_id << std::endl;
#endif
		return (NONE);
	}
	else
	{
		if (_si[_s_id].location[_l_id].cgi_path.empty())
		{
			gen_startLine( 403 );
			return (NONE);
		}
#ifdef _debug_
		std::cout << "Launching cgi" << std::endl;
#endif
		cgi_var_init();
		env = extract_env(_hrx, _si[_s_id]);
		create_tmp_file(&redir_fd[1]);
		if ((redir_pid = go_cgi_fd(&redir_fd, _si[_s_id].location[_l_id].cgi_path, env, _hrx["BODY"][0])) == -1 || (redir_fd[0] == -1 || redir_fd[1] == -1))
		{
			if (redir_fd[0] != -1)
			{
				close(redir_fd[0]);
				redir_fd[0] = -1;
			}
			if (redir_fd[1] != -1)
			{
				close(redir_fd[1]);
				redir_fd[1] = -1;
			}
			redir_pid = -1;
			return (NONE);
		}
		_body = _hrx["BODY"][0];
#ifdef _debug_
		std::cout << "Buffer copied in body" << std::endl;
#endif
		return (CGI_IN);
	}
	return (NONE);
}

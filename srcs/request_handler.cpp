/* 
*      HTTP-message   = start-line
*                      *( ./c	er-field CRLF )
*                      CRLF
*                      [ message-body ]
*
*      start-line     = request-line (client)/ status-line (server)

		http://127.0.0.1:8080/ <- c'est ca qui fait bugger
		curl --resolve test_server_block.com:9090:127.0.0.1 http://test_server_block.com:9090/ -X GET
*/

#include "cgi_handler.hpp"
#include "request_handler.hpp"
#include "status_lines.hpp"

request_handler::request_handler(std::vector<server_info>& server_info) : _si(server_info)
{// BON EN FAIT IL SEMBLE QUE LA SECU NE SOIT PAS LE POINT IMPORTANT DE WEBSERV...
/* const char *array[] = {	"GET", "Host:", "User-Agent:", "Accept:", "Accept-Language:",
												"Accept-Encoding:", "Connection:", "Upgrade-Insecure-Requests:",
												"Cache-Control:", "DNT:", "rol:"};

	for (size_t i = 0; i < sizeof(array) / sizeof(const char*); ++i)
		_hrx[array[i]];
		// _hrx.insert(std::make_pair(array[i], vector<string>()));
#ifdef _debug_
	for (map< string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it)
		cout <<  YELLOW "map it.first : " RESET << it->first << " size() : " << it->second.size() << endl; 
#endif
*/
#ifdef _debug_
	// for (map< string, string>::iterator it = _status.begin(), end = _status.end(); it != end; ++it)
	// cout <<  YELLOW "map it.first : [" RESET << it->first << "] second : [" << it->second << "]" << endl;
#endif
}

request_handler::~request_handler()
{
}

/* cout << distance(mymap.begin(),mymap.find("198765432")); */ // <- Get index of the pair(key_type, mapped_type) TIPS&TRICKS

void request_handler::reader(std::string& rqst)
{
//	const char	*str = client.rqst.c_str();
	size_t	pos;
	string buf_1, buf_2;
	std::string ss_1(rqst);

//	cout << RED "DANS HEADER READER" RESET "\n" << str << endl;
	// LECTURE DE LA START_LINE
	if (!ss_1.empty() && (pos = ss_1.find("\r\n")) != string::npos)
	{
		buf_1 = ss_1.substr(0, pos + 2);
		ss_1 = ss_1.substr(pos + 2);
		std::stringstream ss_2(buf_1);
		while (ss_2 >> buf_1)
			_hrx["A"].push_back(buf_1);
	}
//	for (auto it = _hrx["A"].begin(); it != _hrx["A"].end(); it++)
//		cout << "*it : " << *it << endl;

	// LECTURE DU RESTE DE LA REQUETE
	while ((pos = ss_1.find("\r\n")) != string::npos) {
		buf_1 = ss_1.substr(0, pos + 2);
		ss_1 = ss_1.substr(pos + 2);
		if (buf_1 == "\r\n") { // SI C'EST UNE REQUESTE POST ON STOCK LE BODY POUR USAGE ULTÉRIEUR
			_hrx["BODY"].resize(1, string());
			_hrx["BODY"][0].append(ss_1);
			break ;
		}
		std::stringstream ss_2(buf_1);
		while (ss_2 >> buf_2) {
			// if (_hrx.find(buf_2) != _hrx.end()) {
				string index = buf_2;
				while (ss_2 >> buf_2)
					_hrx[index].push_back(buf_2);
		}
			// }
			// else
			// 	throw (std::runtime_error( "Unkown header field (header_reader) : " + buf_2));
#ifdef _debug_
	// cout << RED << "buf_1.size : "<< buf_1.size() << RESET "[" << buf_1 << "]" << endl;
	// cout << GREEN "ss_2 >> buf_2 : " RESET << "[" << buf_2 << "]" << endl;
	// cout << MAGENTA << "tour" << RESET << endl;
#endif
	}
	set_server_id();
//	client.rqst.clear();
	// cout << RED "APRES GETLINE : " << buf_1 << RESET << endl;
	// this->display();

/*
 	std::cout << "=========================================================" << std::endl;
	for(std::map<std::string, std::vector<std::string> >::iterator it = _hrx.begin(); it != _hrx.end(); it++)
	{
		std::cout << it->first << "=";
		if (it->first == "BODY")
		{
			std::cout << "BODY LEN=" << it->second[0].length() << std::endl;
		}
		for (std::vector<std::string>::iterator i = it->second.begin(); i != it->second.end(); i++)
			std::cout << *i << " ";
		std::cout << std::endl;
	}
	std::cout << "=========================================================" << std::endl; 
	*/
}

int request_handler::choose_method(void)
{
	int	redir_mode;
	gen_startLine( 200 );
	gen_date();
	gen_serv();

	redir_mode = NONE;
// DÉFINI L'INDEX DE LA LOCATION CONCERNÉE (_l_id) & VÉRIFIE QUE LA MÉTHODE INVOQUÉE Y EST PERMISE
	if (resolve_path())
		;
	else if ((ext_id = is_cgi(_hrx["A"], _si[_s_id].cgi_file_types) != -1))
		redir_mode = handle_cgi();
	else if (_hrx["A"][0] == "POST")
		redir_mode = handle_post_rqst();
	else if (_hrx["A"][0] == "GET")
		handle_get_rqst();
	else if (_hrx["A"][0] == "PUT") {
		cout << "Facultatif PUT method not implemented yet\n";
	}
	else if (_hrx["A"][0] == "HEAD") {
		cout << "Facultatif HEAD method not implemented yet\n";
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
			puts( "File successfully deleted" );
			gen_startLine( 204 );
			redir_mode = writer();
			_hrx.clear();
			_htx.clear();
			return (NONE);
		}
	}
	if (redir_mode == NONE)
	{
		file_type();
		gen_CType(string());
		gen_CLength();
		redir_mode = writer();
	}
	_hrx.clear();
	_htx.clear();
	return (redir_mode);
}

int request_handler::writer(void) {
// PAR DEFAULT ON CONSIDÈRE QUE TOUT SE PASSE BIEN ON CHANGE PAR LA SUITE LE STATUS SI UNE EXCEPTION ARRIVE
/* PROBLEM REDONDANT AVEC LA METHODE GET -> VA FALLOIR CHOISIR*/
	int	redir_mode;

	add_all_field(); 
	redir_mode = add_body();
	_hrx.clear();
	_htx.clear();
	return (redir_mode);
}

int	request_handler::cgi_writer()
{
	int	redir_mode;

	gen_startLine( 200 );
	gen_CLength();
	add_all_field(); 
	std::cout << "All fields added" << std::endl;
	redir_mode = add_body();
	std::cout << "Body added" << std::endl;
	_hrx.clear();
	_htx.clear();
	return (redir_mode);
}

	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */

void	request_handler::gen_startLine(size_t ret_code)
{
	std::stringstream	ss;

	if (_htx["A"].empty())
		_htx["A"] = std::vector<std::string>();
	if (_htx["A"].size() != 3)
		_htx["A"].resize(3, string());
	_htx["A"][0] = "HTTP/1.1 "; // version (static)
	ss << ret_code;
	_htx["A"][1] = ss.str(); // status code (dynamic) -> 200
	_htx["A"][2] = ret_string(ret_code); // status msg (dynamic) -> OK
	_htx["A"][2] += "\r\n";
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

	// _response.append(date);
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
	if (ext.empty())
		ext = _hrx["A"][1].substr(_hrx["A"][1].find_last_of(".") + 1);
#ifdef _debug_
	cout << "file ext asked : " << ext << endl;
#endif
// Content type : https://developer.mozilla.org/fr/docs/Web/HTTP/Basics_of_HTTP/MIME_types
	_htx["Content-Type"].clear();
	if ( ext == "/" || ext == "html" ) // Default file PROBLEM ?
		_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
	else if ( ext == "css" ) // Default file PROBLEM ?
	{
		_htx["Content-Type"].push_back("Content-Type: text/css\r\n");
//		_htx["Connection"].push_back("Connection: close\r\n");
	}
	else if( ext == "ico" || ext == "png" || ext == "jpeg" || ext == "webp" || ext == "gif" || ext == "bmp" )
		_htx["Content-Type"].push_back("Content-Type: image\r\n");
	else if( ext == "ogg" || ext == "wav" || ext == "midi" || ext == "mpeg" || ext == "webm" )
		_htx["Content-Type"].push_back("Content-Type: audio\r\n");
	else if( ext == "ogg" || ext == "mp4" || ext == "webm" )
		_htx["Content-Type"].push_back("Content-Type: video\r\n");
	else // DEFAULT PAREIL QUE L EPREMIER À ARRANGER :) POUR LE TOI DU FUTUR BISOU
		_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
		// _htx["Content-Type"].push_back("Content-Type: application/octet-stream\r\n");
}

// génération du field Content-Length et d'un status d'erreur (413) si length > max_file_size
void	request_handler::gen_CLength()
{/* PROBLEME */ // http://127.0.0.1:8080/test les gif 404 s'affiche http://127.0.0.1:8080/test/ <- '/' non
/* PROBLEM */ // http://127.0.0.1:8080/downloads/ n'affiche pas le poulpe -> les autre requetes sont faites sur le folder alors que le path du html est neutre, mais ça ça marche : http://127.0.0.1:8080/downloads 
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
/* 	if (!_si[_s_id].location[_l_id].max_file_size.empty() // POUR LES REQUEST ENTITY TOO LARGE SUREMENT A VIRER D'ICI
		&& atoi(ss.str().c_str()) > atoi(_si[_s_id].location[_l_id].max_file_size.c_str()))
		gen_startLine( 413 ); */
}

	/* FUNCTION SECONDAIRE : UTILITAIRES */

// reconnait quel server_virtuel va traiter la requete et initialise _s_id
void request_handler::set_server_id(void)
{
	size_t colon_pos = _hrx["Host:"][0].find_first_of(":");
	string host(_hrx["Host:"][0].substr(0,colon_pos));
	// PROBLEM : SPARADRAP
//	if (host == "127.0.0.1") host = "localhost";
	string port(_hrx["Host:"][0].substr(colon_pos +1));
	for (size_t i = 0; i < _si.size(); ++i)
		if ( (_si[i].host == host || _si[i].server_name == host) && _si[i].port == port ) {
			_s_id = i;
			cout << RED "_s_id : " RESET << _s_id << endl;
			return ;
		}
// SI LE HOST:PORT N'A PAS ETE TROUVE ON SELECT LE PREMIER SERVER CORRESPONDANT
	for (size_t i = 0; i < _si.size(); ++i)
		if (_si[i].port == port) {
			_s_id = i;
			break;
		}
#ifdef _debug_
			cout << "host : " << host << ", port : " << port << ", id : " << _s_id << endl;
#endif
}

/* ◦ Le premier serveur pour un host :port sera le serveur par défaut pour cet
host :port (ce qui signifie qu’il répondra à toutes les requêtes qui n’appar-
tiennent pas à un autre serveur) */



void request_handler::handle_get_rqst(void)
{
	// gen_CType(string());
	// gen_CLength();
	// add_all_field();
	// add_body();
}

int request_handler::multipart_form(string& boundary, string& msg)	{
	size_t	pos, end;
	string	tmp, buf, path;
	ofstream	out;

	pos = msg.find(boundary);
	if (pos != string::npos)
	{
		msg = msg.substr(pos + boundary.length());
		if (msg.substr(0, 2) == "\r\n")
			msg = msg.substr(2);
		else if (msg.substr(0, 2) == "--")
		{
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
//		std::cout << "tmp = " << tmp << std::endl;
		if ((pos = tmp.find("filename=\"")) != string::npos)
		{
			buf = tmp.substr(pos + strlen("filename=\""), tmp.find("\r\n"));
			path = buf.substr(0, buf.find("\""));
		}
		if (msg.substr(0, 2) == "\r\n")
			msg = msg.substr(2);
		buf.clear();
		end = msg.find(boundary);
		if ((pos = msg.find(boundary + "--")) == end && pos != string::npos)
		{
			buf.append(msg.substr(0, pos));
			_body = buf;
			msg = msg.substr(pos + (boundary + "--").length());
			redir_fd = open(path.c_str(), O_CREAT | O_WRONLY, S_IRWXU);
			if (redir_fd == -1)
				return (NONE);
			else
				return (WRITE);
		}
		else
		{
			buf.append(msg.substr(0, end));
			msg = msg.substr(end + (boundary).length());
		}
	}
	return (NONE);
}

int request_handler::handle_post_rqst(void) 
{
	int			redir_mode;
	std::string	tmp;
	std::string	boundary;

	redir_mode = NONE;
	if (_hrx.find("Content-Type:") != _hrx.end())
		cout << "_hrx['Content-Type:'].size() : " << _hrx["Content-Type:"].size() << " :" << endl;
	for (std::vector<std::string>::iterator i = _hrx["Content-Type:"].begin(); i != _hrx["Content-Type:"].end(); i++)
		cout << "[" << *i << "]" << endl;

	if (_hrx["Content-Length:"].empty() && _hrx["Transfer-Encoding:"].empty())
	{
		gen_startLine( 400 ); 
		return (NONE);
	}

// POUR LIMITER LA TAILLE DU BODY DU CLIENT => JE NE SAIT PAS ENCORE COMMENT GET LA LOCATION CONCERNÉE
	// if (_hrx.find("Content-Length:") != _hrx.end() && atoi(_hrx.find("Content-Length:")->second) > _si[_s_id]. )
	if (_hrx["Content-Type:"].size() > 1)
		boundary = _hrx["Content-Type:"][1].substr(strlen("boundary=--"));
//	std::cout << boundary << std::endl;
	cout << endl << "BODY : " << endl;
	// for (auto i = _hrx["BODY"].begin(); i != _hrx["BODY"].end(); i++)
	// 	cout << "[" << *i << "]" << endl;
//	cout << _hrx["BODY"][0] << endl;

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
				{
					for (map<string, vector<string> >::iterator it = _hrx.begin(); it != _hrx.end(); it++)
					{
						if (it->first != "BODY")
						{
							if (it->first != "A")
							{
								_response.append(it->first);
								_response.append(" ");
							}
							for (vector<string>::iterator i = it->second.begin(); i != it->second.end(); i++)
							{
								if (it->first == "A" && *i == "POST")
								{
									_response.append("GET");
									_response.append(" ");
								}
								else if (it->first != "Content-Length:" && it->first != "Content-Type:")
								{
									_response.append(*i);
									_response.append(" ");
								}
							}
							_response.append("\r\n");
						}
					}
					_response.append("\r\n");
					return (redir_mode);
				}
				resolve_path();
			}
		}
	}
	return (redir_mode);
}

// Permet de séléctionner la location qui partage le plus avec l'url comme le fait nginx,
// si l'url a plusieurs niveau de dossiers, choisi la loc la plus adaptée
// identifie l'index de la location correspondante à l'url spcécifiée (_l_id)
int	request_handler::resolve_path()
{
	size_t len;

#ifdef _debug_
	cout << GREEN "DS RESOLVE_PATH [" + _hrx["A"][1] + "], _s_id : " << _s_id <<  " _path (cleared afterward): " << _path << endl;
#endif
	_path.clear();
// REMOVE MULTIPLE '/' AND THE '/' AT URL'S END
	clean_url(_hrx["A"][1]);
// MANOUCHERIE A VIRER POUR RECUP LES PAGES D'ERREUR SANS BOUCLE SUR 4XX.HTML SI 405
	if (_hrx["A"][1].find("/error_pages/", 0,13) == 0) {
		_path = (_hrx["A"][1][0] == '/' ? "files" + _hrx["A"][1] : "files/" + _hrx["A"][1]);
		return true;
	}

	_l_id = 0;
	int index = _si[_s_id].location.size() - 1;
	len = 0;
	for (vector<locati_info>::reverse_iterator it = _si[_s_id].location.rbegin(); it != _si[_s_id].location.rend(); ++it, --index)
		if ((_hrx["A"][1].find( it->location.c_str(), 0 , it->location.size()) == 0 && it->location.size() > len) || (!len && !index)) // || si on a rien trouvé la location[0] est le default
		{
			// S'IL Y A UNE DIRECTIVE_RETURN À L'INTERIEURE DE LA LOCATION
			if (!it->retour.empty()) {
				for (vector<locati_info>::iterator it2 = _si[_s_id].location.begin(); it2 != _si[_s_id].location.end(); ++it2)
					if (it2->location == it->retour.back()) {
						cout << RED " it->retour[1] :" RESET +  it->retour[1] << endl;
						string::const_iterator c_it = it->retour[0].begin();
						while (c_it != it->retour[0].end() && std::isdigit(*c_it))
							++c_it;
						if ( !it->retour[0].empty() && c_it == it->retour[0].end())
						{
							stringstream	ss;
							size_t			ret;

							ss.str() = it->retour[0];
							ss >> ret;
							gen_startLine( ret );
						}

						_path = it2->root[it2->root.size() - 1] == '/' ? it2->root : it2->root + "/";
						_l_id = it2 - _si[_s_id].location.begin();
						break ;
					}
			} // SINON
			else {
				_path = it->root[it->root.size() - 1] == '/' ? it->root : it->root + "/";
				_l_id = index;
			}
			// _path += it->location.back() == '/' ? it->location : it->location + "/";
			_path += _hrx["A"][1].substr(it->location.size());
			len = it->location.length();
			// if (it->location.size() == _hrx["A"][1].size()) // Mnt ajout de l'index.html mis ds file_type si necessaire
			// 	_path += _si[_s_id].location[_l_id].index;
		}
#ifdef _debug_
	cout << BLUE "path : " RESET << _path << endl;
#endif
// AFFACER LES '/' EN PRÉFIXE (POUR OUVRIR DEPUIS LA RACINE DE NOTRE DOSSIER ET PAS DEPUIS LA RACINE MERE)
	while (_path[0] == '/')
		_path.erase(_path.begin());
	clean_url(_path);
#ifdef _debug_
	cout << BLUE "resolved_path : " RESET << _path << endl;
	cout << "location [" << _l_id << "] : " + _si[_s_id].location[_l_id].location << endl;
#endif
// VERIFIE SI LA MÉTHODE DS LA LOCATION CONCERNÉE EST AUTORISÉE

	bool allowed = false;
	for (size_t i = 0; i < _si[_s_id].location[_l_id].allowed_method.size(); ++i)
		if (_si[_s_id].location[_l_id].allowed_method[i] == _hrx["A"][0])
			allowed = true;
	if (!allowed)
		gen_startLine( 405 );
	return allowed ? 0 : 1;  /*PROBLEM  oN SAIT PAS TROP CE QU'ON FAIT LÀ... (double return) */
}

// Détecte si c'est un fichier normal ou s'il n'existe pas (maj de la statut-line si besoin)
// Si c'est un dossier check l'autoindex ou la presence d'une directive index et affiche le contenu du dossier ou ajoute le fichier index respectivement
// Si erreur lors de l'ouverture du fichier renvoie le _path sur les pages d'erreurs
int request_handler::file_type()
{
	cout << GREEN "DS FILE_TYPE()" RESET <<  " _path : " << _path << endl;
	struct stat sb;

	bzero(&sb, sizeof(sb));
	if (lstat(_path.c_str(), &sb) == -1)
		perror("lstat");

	switch (sb.st_mode & S_IFMT) {
		// case S_IFBLK:  printf("block device\n");			break;
		// case S_IFCHR:  printf("character device\n");		break;
		case S_IFDIR:  printf("directory\n");
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
/* 		case S_IFIFO:  printf("FIFO/pipe\n");				break;
		case S_IFLNK:  printf("symlink\n");					break; */
		case S_IFREG:  printf("regular file\n");			break;
/* 		case S_IFSOCK: printf("socket\n");					break; */
		default:
			if (atoi(_htx["A"][1].c_str()) < 400)
				gen_startLine( 404 );
			printf(RED "unknown path : %s\n" RESET, _path.c_str());
			break;
	}
// AIGUILLE LE PATH SUR LA PAGE D'ERREUR CORRESPONDANTE
	if (atoi(_htx["A"][1].c_str()) >= 400)
		_path = _si[_s_id].error_page.empty() || _si[_s_id].error_page.find("files/error_pages") != string::npos ? "files/error_pages/4xx.html" : _si[_s_id].error_page + _htx["A"][1] + ".html";
	printf("_path : %s\n", _path.c_str());
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
	    	std::cout << epdf->d_name << std::endl;
		}
	closedir(dpdf);

	fstream autoindex_file("configuration_files/autoindex.html");
	if (!autoindex_file.is_open())
		throw (std::runtime_error("Couldn't open : configuration_files/autoindex.html"));
// BUFFERISE LE TEMPLATE HTML POUR Y AJOUTER LE CONTENU DU DOSSIER
	std::stringstream buffer;
	buffer << autoindex_file.rdbuf();
	_body = buffer.str();
	autoindex_file.close();
	// cout << "_hrx['A'][1]" << _hrx["A"][1] << " _path : "  << _path << endl;
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
	cout << BLUE "Response Headers (add_all_field()) :\n" RESET << _response << endl;
}

// Ajout du fichier ou du body À LA SUITE des header dans response
int request_handler::add_body()
{
/* 	if (_htx["A"][1] == "413") // PLUS DE REQUETE TROP LONGUE POUR LA REPONSE AU CLIENT
		return ; */
	if (!_body.empty()) {
		_response += _body;
		_body.clear();
		return (NONE);
	}
	std::cout << "body appended" << std::endl;
// S'IL S'AGIT D'UN GET OU D'UN POST ON JOINS LE FICHIER
	if (!_hrx["A"].empty() && (_hrx["A"][0] == "GET" || _hrx["A"][0] == "POST")) {
		cout << RED "File written !" RESET  << endl;
		redir_fd = open(_path.c_str(), O_RDONLY | O_NONBLOCK);
		if (redir_fd == -1)
		{
			std::cout << "Open error" << std::endl;
			return (NONE);
		}
//		std::cout << _body << std::endl;
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

std::string &request_handler::get_response(void) {return _response;}

std::string &request_handler::get_body(void) {return _body;}

void request_handler::set_body(const string& str)	{
	_body = str;
}

int request_handler::get_redir_fd(void) {return redir_fd;}

	/* FUNCTION DE DEBUG */

void request_handler::display(void) {
	cout << GREEN ITALIC UNDERLINE "DISPLAY HEADER INFORMATION" RESET GREEN " :" RESET << endl;
	for (map<string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it) {
		cout << it->first << " ";
		for (size_t i = 0; i < it->second.size(); ++i)
			cout << it->second[i] << ", ";
		cout << endl;
	}
}

std::vector<std::string> request_handler::extract_env(std::map<std::string, std::vector<std::string> >& mp,const server_info& _s)
{
	std::vector<std::string>	env;
	std::string			tmp;
	std::string			buf(mp["A"][1]);
	std::string			var;

	tmp =  "REDIRECT_STATUS=CGI";
	env.push_back(tmp);
	tmp = "SERVER_SOFTWARE=";
	for (size_t j = 0; j < _s.server_name.size(); ++j)
		tmp += _s.server_name[j];
	env.push_back(tmp);
	tmp = "SERVER_NAME=";
	tmp += _s.host;
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
	tmp = "SCRIPT_NAME=";
	if (!mp["Script-Name"].empty())
	{
		for (size_t j = 0; j < mp["Script-Name"].size(); ++j)
			tmp+= mp["Script-Name"][j];
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
	tmp = "HTTP_ACCEPT=";
	if (!mp["Accept:"].empty())
	{
		for (size_t j = 0; j < mp["Accept:"].size(); ++j)
			tmp+= mp["Accept:"][j];
	}
	env.push_back(tmp);
	tmp = "HTTP_ACCEPT_LANGUAGE=";
	if (!mp["Accept-Language:"].empty())
	{
		for (size_t j = 0; j < mp["Accept-Language:"].size(); ++j)
			tmp+= mp["Accept-Language:"][j];
	}
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
	return (env);
}

void	request_handler::clean_body()
{
	std::string	tmp;
	std::string	index;
	size_t	pos;
	
	while ((pos = _body.find("\r\n")) != std::string::npos)
	{
		tmp = _body.substr(0, pos + 2);
		_body = _body.substr(pos + 2);
		if (tmp == "\r\n")
			break ;
		if ((pos = tmp.find(":")) != std::string::npos)
		{
			index = tmp.substr(0, pos);
//			cout << "index=" << index << endl;
			if (index != "X-Powered-By")
				_htx[index].push_back(tmp.substr(0, tmp.find("\r\n") + 2));
		}
	}
}

void	request_handler::cgi_var_init()	{
	size_t				len, pos;
	std::string			var;

	//CGI variables initialisation
	if ((len = _path.find("?")) != std::string::npos)
	{
		var = _path.substr(len + 1);
		_path = _path.substr(0, len);
	}
	std::cout << "PATH=" << _path << std::endl;
	_hrx.insert(std::make_pair("Path-Translated", std::vector<std::string>()));
	_hrx["Path-Translated"].push_back(_path);
	_hrx.insert(std::make_pair("Query-String", std::vector<std::string>()));
	_hrx["Query-String"].push_back(var);
	var.clear();
	_hrx.insert(std::make_pair("Path-Info", std::vector<std::string>()));
	pos = _path.find(_si[_s_id].cgi_file_types[ext_id]);
	if (pos != std::string::npos)
	{
		var = _path.substr(pos + 4);
		_path = _path.substr(0, pos + 4);
	}
	if (!var.empty() && (pos = var.find("/")) != std::string::npos)
		var = var.substr(pos + 1);
	_hrx["Path-Info"].push_back(var);
	var.clear();
	_hrx.insert(std::make_pair("Script-Name", std::vector<std::string>()));
	var = (_hrx["A"][1][0] == '/' ? _hrx["A"][1].substr(1) : _hrx["A"][1]);
	_hrx["Script-Name"].push_back(var);
	_hrx.insert(std::make_pair("Document-Root", std::vector<std::string>()));
	pos = _path.find_last_of("/");
	var = _path.substr(0, pos);
	if (var.substr(0, 2) == "./")
		var = var.substr(2);
	_hrx["Document-Root"].push_back(var);
	var.clear();
}

int	request_handler::handle_cgi(void)
{
	std::vector<std::string>	env;

	//HERE!
	if (_s_id == -1)
	{
		std::cout << "Invalid server id: " << _s_id << std::endl;
		return (NONE);
	}
	else
	{
	//EN CHANTIER !!!
		if (_si[_s_id].cgi_path.empty())
		{
			gen_startLine( 403 );
			return (NONE);
		}
		std::cout << "Launching cgi" << std::endl;
		cgi_var_init();
		env = extract_env(_hrx, _si[_s_id]);
		redir_fd = go_cgi(_si[_s_id].cgi_path, _hrx["BODY"], env);
		if (redir_fd == -1)
			return (NONE);
		return (CGI_OUT);
	//EN CHANTIER !!!
	}
	return (NONE);
}

void	request_handler::clean(void)	{
	_hrx.clear();
	_htx.clear();
	//_status.clear();
	_path.clear();
	_body.clear();
	_response.clear();
}

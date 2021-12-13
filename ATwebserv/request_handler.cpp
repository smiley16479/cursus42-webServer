/* 
*      HTTP-message   = start-line
*                      *( header-field CRLF )
*                      CRLF
*                      [ message-body ]
*
*      start-line     = request-line (client)/ status-line (server)

		http://127.0.0.1:8080/ <- c'est ca qui fait bugger
		curl --resolve test_server_block.com:9090:127.0.0.1 http://test_server_block.com:9090/ -X GET
*/

#include "request_handler.hpp"

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

// REMPLI LA MAP _STATUS POUR LA STATUS LIGNE (TX) HTML
	ifstream fs("configuration_files/HTML_status_msg.txt");
	if (!fs.is_open()) 
		throw (std::runtime_error("Unkown file : configuration_files/HTML_error_msg.txt"));

	string buf_1, buf_2;
	while (std::getline(fs, buf_1)) {
		if (buf_1[0] == '#' || buf_1[0] == '\0')
			continue ;
		std::stringstream ss_2(buf_1);
		ss_2 >> buf_1; 
		std::getline(ss_2, buf_2);
		_status[buf_1].append(buf_2.substr(0, buf_2.find_first_of('\t')));
	}
#ifdef _debug_
	// for (map< string, string>::iterator it = _status.begin(), end = _status.end(); it != end; ++it)
	// cout <<  YELLOW "map it.first : [" RESET << it->first << "] second : [" << it->second << "]" << endl;
#endif
}

request_handler::~request_handler()
{
}

/* cout << distance(mymap.begin(),mymap.find("198765432")); */ // <- Get index of the pair(key_type, mapped_type) TIPS&TRICKS

void request_handler::reader(const char *str)
{
	string buf_1, buf_2;
	std::stringstream ss_1(str);
	cout << RED "DANS HEADER READER" RESET "\n" << str << endl;
	// LECTURE DE LA START_LINE
	if (std::getline(ss_1, buf_1)) {
		std::stringstream ss_2(buf_1);
		while (ss_2 >> buf_1)
			_hrx["A"].push_back(buf_1);
	}
	for (auto it = _hrx["A"].begin(); it != _hrx["A"].end(); it++)
		cout << "*it : " << *it << endl;

	// LECTURE DU RESTE DE LA REQUETE
	while (std::getline(ss_1, buf_1)) {
		if (buf_1[0] == '\r') { // SI C'EST UNE REQUESTE POST ON STOCK LE BODY POUR USAGE ULTÉRIEUR
			_hrx["BODY"].resize(1, string());
			while (std::getline(ss_1, buf_1))
				_hrx["BODY"][0].append(buf_1);
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
	// cout << RED "APRES GETLINE : " << buf_1 << RESET << endl;
	// this->display();
}

void request_handler::writer(void) {

// PAR DEFAULT ON CONSIDÈRE QUE TOUT SE PASSE BIEN ON CHANGE PAR LA SUITE LE STATUS SI UNE EXCEPTION ARRIVE
	gen_startLine( _status.find("200") );
	gen_date();
	gen_serv();

	if (_hrx["A"][0] == "GET")
		handle_get_rqst();
	else if (_hrx["A"][0] == "POST")
		handle_post_rqst();
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
			gen_startLine( _status.find("404") ); //  IF NOT FOUND -> 404
		}
		else
			puts( "File successfully deleted" );
	}
	_hrx.clear();
	_htx.clear();
}

	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */

void	request_handler::gen_startLine(std::map<string, string>::iterator status)
{
	if (_htx["A"].size() != 3)
		_htx["A"].resize(3, string());
	_htx["A"][0] = "HTTP/1.1 "; // version (static)
	_htx["A"][1] = status->first; // status code (dynamic) -> 200
	_htx["A"][2] = status->second; // status msg (dynamic) -> OK
	_htx["A"][2] += "\r\n";
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
	_htx["Date"].push_back(date);
}

void	request_handler::gen_serv() /* PROBLEM : S'IL N'Y A PAS DE CHAMP Server DS LA REQUETE TU SEGV PAR EX POUR UN POST DS GD TAILLE*/
{
	if (_htx["Server"].size() != 3)
		_htx["Server"].resize(3, string());
	_htx["Server"][0] = "Server: ";// HEADER_LABEL
	_htx["Server"][1] = _hrx["Host:"][0];// IP & HOST
	_htx["Server"][2] = "\r\n";

}

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

// génération du field content-length
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
}

	/* FUNCTION SECONDAIRE : UTILITAIRES */

// reconnait quel server_virtuel va traiter la requete et initialise _s_id
void request_handler::set_server_id(void)
{
	size_t colon_pos = _hrx["Host:"][0].find_first_of(":");
	string host(_hrx["Host:"][0].substr(0,colon_pos));
	// PROBLEM : SPARADRAP
	if (host == "127.0.0.1") host = "localhost";
	string port(_hrx["Host:"][0].substr(colon_pos +1));
	for (int i = 0; i < _si.size(); ++i)
		if ( (_si[i].host == host || _si[i].server_name == host) && _si[i].port == port ) {
			_s_id = i;
			cout << RED "_s_id : " RESET << _s_id << endl;
			return ;
		}
// SI LE HOST:PORT N'A PAS ETE TROUVE ON SELECT LE PREMIER SERVER CORRESPONDANT
	for (int i = 0; i < _si.size(); ++i)
		if (_si[i].port == port)
			_s_id = i; // PROBLEM ?
#ifdef _debug_
			cout << "host : " << host << ", port : " << port << ", id : " << _s_id << endl;
#endif
}

/* ◦ Le premier serveur pour un host :port sera le serveur par défaut pour cet
host :port (ce qui signifie qu’il répondra à toutes les requêtes qui n’appar-
tiennent pas à un autre serveur) */



void request_handler::handle_get_rqst(void)
{
	gen_CType(string());
	verify_file_openess();
	gen_CLength(); // Add ContentLength and Body
	add_all_field();
	add_body();
}

void request_handler::handle_post_rqst(void) 
{
	if (_hrx.find("Content-Type:") != _hrx.end())
		cout << "_hrx['Content-Type:'].size() : " << _hrx["Content-Type:"].size() << " :" << endl;
	for (auto i = _hrx["Content-Type:"].begin(); i != _hrx["Content-Type:"].end(); i++)
		cout << "[" << *i << "]" << endl;

// POUR LIMITER LA TAILLE DU BODY DU CLIENT => JE NE SAIT PAS ENCORE COMMENT GET LA LOCATION CONCERNÉE
	// if (_hrx.find("Content-Length:") != _hrx.end() && atoi(_hrx.find("Content-Length:")->second) > _si[_s_id]. )
	string boundary = _hrx["Content-Type:"][1].substr( _hrx["Content-Type:"][1].find_last_of('-') + 1, string::npos);
	cout << "boundary : " << boundary << endl;
	cout << endl << "BODY : " << endl;
	// for (auto i = _hrx["BODY"].begin(); i != _hrx["BODY"].end(); i++)
	// 	cout << "[" << *i << "]" << endl;
	cout << _hrx["BODY"][0] << endl;

	// En cas de body plus long qu'autorisé -> 413 (Request Entity Too Large) 
	if ( _hrx["BODY"][0].size() > atoi(_si[_s_id].max_file_size.c_str()) ) {
		gen_startLine( _status.find("413") ); 
		return ;
	}
	// sinon on execute les cgi ?
}

// vérification de la bonne ouverture du fichier (maj de la statut-line si besoin)
// Si erreur lors de l'ouverture du fichier renvoie fichier ouvert sur les pages d'erreurs
// Le curseur du fichier pointe sur sa fin pour que fs.tellg() donne sa taille à Content-Length
void	request_handler::verify_file_openess()
{
	_path.clear();
	if (resolve_path()) // Resolve _path s'occupe d'écrire le contenu des dossiers aussi
		return ;

// Ancienne version : rmplacée par l'ouverture du fichier d'errueur ds file_type()
	// // ifstream fs(_path.c_str(), std::ifstream::binary | std::ifstream::ate);
	// ifstream fs(_path, std::ifstream::binary /* | std::ifstream::ate */);
	// if (!fs.is_open()) { // SI LE FICHIER N'EST PAS TROUVÉ ALORS ON ENVOIE LES PAGES D'ERREUR
	// 	gen_startLine( _status.find("404") );
	// 	if (_si[_s_id].error_page.empty()) // CHOISI LE FICHIER D'ERREUR PAR DEFAULT OU CELUI DE LA CONF
	// 		fs.open("files/error_pages/error_4xx.html",  std::ifstream::binary | std::ifstream::ate);
	// 	else
	// 		fs.open(_si[_s_id].error_page + "error_4xx.html",  std::ifstream::binary | std::ifstream::ate);
	// 	if (!fs.is_open())
	// 		throw (std::runtime_error( "Unkown _path (header_writer) : error_4xx.html"));
	// 	if (_htx["Content-Type"].empty())
	// 		_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
	// 	else
	// 		_htx["Content-Type"][0] = "Content-Type: text/html; charset=utf-8\r\n";
	// }
}

// Permet de séléctionner la location qui partage le plus avec l'url (comme le fait nginx)
int	request_handler::resolve_path()
{
// REMOVE MULTIPLE '/' AND THE '/' AT URL'S END
	clean_url(_hrx["A"][1]);
// INDEX DE LA LOCATION CONCERNÉE (loc_id)
	int loc_id = location_lookup();
// VERIFIE SI LA MÉTHODE DS LA LOCATION CONCERNÉE EST AUTORISÉE
	bool allowed = false;
	for (int i = 0; i < _si[_s_id].location[loc_id].allowed_method.size(); ++i)
		if (_si[_s_id].location[loc_id].allowed_method[i] == _hrx["A"][0])
			allowed = true;
	if (!allowed)
		gen_startLine( _status.find("405") );
#ifdef _debug_
	cout << BLUE ", _path : " RESET << _path << endl;
#endif
// AFFACER LES '/' EN PRÉFIXE (POUR OUVRIR DEPUIS LA RACINE DE NOTRE DOSSIER ET PAS DEPUIS LA RACINE MERE)
	while (_path[0] == '/')
		_path.erase(_path.begin());
	clean_url(_path);

#ifdef _debug_
	cout << BLUE "resolved_path : " RESET << _path << endl;
#endif
	return file_type(loc_id);
}


// si l'url a plusieurs niveau de dossiers : choisir le plus adapté
int request_handler::location_lookup_2() /* RELIQUAT */
{
// URL == A LA TOTALITE DE LA REQUETE, URI == A LA DERNIERE PORTION APRES LE DERNIER '/'
	string url(_hrx["A"][1]);
	cout << GREEN "DS LOCATION_LOOKUP URL : " RESET << url + " [" + _hrx["A"][1] + "], _s_id : " << _s_id << endl;
	string uri(url.substr(url.find_last_of("/")));
	for (int pos_cut; url.size();) {
		for (size_t i = _si[_s_id].location.size(); i ; --i) {
			if (url == _si[_s_id].location[i - 1].location) {
				_path = _si[_s_id].location[i - 1].root.back() == '/' ? _si[_s_id].location[i - 1].root : _si[_s_id].location[i - 1].root + "/";
				// _path += _si[_s_id].location[i - 1].location.back() == '/' ? _si[_s_id].location[i - 1].location : _si[_s_id].location[i - 1].location + "/"; // On ne fait pas comme nginx finalement
				_path += _hrx["A"][1].substr(url.size());
// SI LA LOCATION.SIZE() == _hrx["A"][1].SIZE() ALORS ON DOIT AFFECTER L'INDEX.HTML (OR WHATEVER) AU PATH
				if (_si[_s_id].location[i - 1].location.size() == _hrx["A"][1].size()) {
					cout << GREEN "DS LOCATION_LOOKUP IF" RESET << endl;
					_path += _si[_s_id].location[i - 1].index;
				}
cout << GREEN "DS LOCATION_LOOKUP  (url == _si[_s_id].location[i - 1].location) URL : " RESET + url + " location : " + _si[_s_id].location[i - 1].location << endl; 
				return i - 1;
			}
		}
		cout << MAGENTA "url : " RESET "[" << url << "]" << endl;
		pos_cut = url.find_last_of("/");
		url.resize(pos_cut);
		cout << GREEN "url : " RESET << url << endl;
	}
	if (url.empty()) {
		_path = _si[_s_id].location[0].root.back() == '/' ? _si[_s_id].location[0].root : _si[_s_id].location[0].root + "/";
		// _path += _si[_s_id].location[0].location.back() == '/' ? _si[_s_id].location[0].location : _si[_s_id].location[0].location + "/"; // On ne fait pas comme nginx finalement
		_path += _hrx["A"][1] == "/" ? _si[_s_id].location[0].index : _hrx["A"][1];
		cout << BLUE "EMPTY _path : " RESET << _path << endl;
	}
	return 0;
}

// si l'url a plusieurs niveau de dossiers : choisir le plus adapté
int request_handler::location_lookup()
{
#ifdef _debug_
	cout << GREEN "DS LOCATION_LOOKUP [" + _hrx["A"][1] + "], _s_id : " << _s_id <<  " _path : " << _path << endl;
#endif

	int id = 0, index = _si[_s_id].location.size() - 1;
	size_t len = 0;
	for (vector<locati_info>::reverse_iterator it = _si[_s_id].location.rbegin(); it != _si[_s_id].location.rend(); ++it, --index)
		if ((_hrx["A"][1].find( it->location.c_str(), 0 , it->location.size()) == 0 && it->location.size() > len) || (!len && !index)) // || si on a rien trouvé la location[0] est le default
		{
			_path = it->root.back() == '/' ? it->root : it->root + "/";
			// _path += it->location.back() == '/' ? it->location : it->location + "/";
			_path += _hrx["A"][1].substr(it->location.size());
			len = it->location.length();
			id = index;
			if (it->location.size() == _hrx["A"][1].size() && !it->index.empty())
				_path += it->index;
		}
#ifdef _debug_
	cout << GREEN "DS LOCATION_LOOKUP : " RESET "location [" << id << "] : " + _si[_s_id].location[id].location << endl;
#endif
	return (id);
}

// Détecte si c'est un dossier ou un fichier normal
int request_handler::file_type(int loc_id)
{
	struct stat sb = {0}; // à la place de : bzero(&sb, sizeof(sb));
	if (lstat(_path.c_str(), &sb) == -1) {
		perror("lstat");
		// exit(EXIT_FAILURE);
	}
	switch (sb.st_mode & S_IFMT) {
		// case S_IFBLK:  printf("block device\n");            break;
		// case S_IFCHR:  printf("character device\n");        break;
		case S_IFDIR:  printf("directory\n");
			if (_si[_s_id].location[loc_id].autoindex == "on") {
// PROBLEM
				generate_folder_list();
				return 1;
			}
			_path = "./files/if_folder.html";                  break;
		// case S_IFIFO:  printf("FIFO/pipe\n");               break;
		// case S_IFLNK:  printf("symlink\n");                 break;
		case S_IFREG:  printf("regular file\n");               break;
		// case S_IFSOCK: printf("socket\n");                  break;
		default:
			gen_startLine( _status.find("404") );
			printf(RED "unknown path...\n" RESET);
			_path = (_si[_s_id].error_page.empty() ? "./files/error_pages/" : _si[_s_id].error_page) + "error_4xx.html";
			break;
	}
	if (atoi(_htx["A"][1].c_str()) >= 400) /* Faire en sorte de changer le png */
		_path = (_si[_s_id].error_page.empty() ? "./files/error_pages/" : _si[_s_id].error_page) + "error_4xx.html";
	printf("_path : %s\n", _path.c_str());
	return 0;
}

// execute le script perl er pipe sont résultat ds _body
void request_handler::generate_folder_list()
{
	int fd[2], pid;
	if (pipe(fd) == -1 || (pid = fork()) == -1)
		throw runtime_error("pipe || fork failed");
	if (pid == 0) {// Child
		char const *argv[] = {"files/cgi/perlFolderLister.pl", _path.c_str(), NULL};
		close(fd[0]);	/* Close unused read end */
		dup2(fd[1], STDOUT_FILENO);
		if (execv(*argv, (char *const *)argv) == -1)
			_exit(EXIT_FAILURE);
		close(fd[1]);
	}
	else {
		close(fd[1]);          /* Close unused write end */
		char buf[1000];
		int n;
		_body.clear();
		while ((n = read(fd[0], buf, 999))) {
			buf[n] = '\0';
			_body.append(buf);
		}
		gen_CType("html");
		// gen_CType("html");
		cout <<  CYAN "folder_ response" RESET << _body  <<  CYAN "path_ response" RESET << _path << endl;
		close(fd[0]);          /* Reader will see EOF */
		wait(NULL);            /* Wait for child */
		return ;
	}
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
void request_handler::add_body()
{
	if (!_body.empty()) {
		_response += _body;
		_body.clear();
		return ;
	}

	if (_hrx["A"][0] == "GET") { // S'IL S'AGIT D'UN GET ON JOINS LE FICHIER
		cout << RED "File written !" RESET  << endl;
		ifstream fs(_path);
		_response.append((istreambuf_iterator<char>(fs)),
						 (istreambuf_iterator<char>() ));
	}
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

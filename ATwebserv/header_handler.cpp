/* 
*      HTTP-message   = start-line
*                      *( header-field CRLF )
*                      CRLF
*                      [ message-body ]
*
*      start-line     = request-line (client)/ status-line (server)
*/

#include "header_handler.hpp"

header_handler::header_handler(std::vector<server_info>& server_info) : _si(server_info)
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

header_handler::~header_handler()
{
}

/* cout << distance(mymap.begin(),mymap.find("198765432")); */ // <- Get index of the pair(key_type, mapped_type) TIPS&TRICKS

void header_handler::reader(const char *str)
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

void header_handler::writer(void) {

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

void	header_handler::gen_startLine(std::map<string, string>::iterator status)
{
	if (_htx["A"].size() != 3)
		_htx["A"].resize(3, string());
	_htx["A"][0] = "HTTP/1.1 "; // version (static)
	_htx["A"][1] = status->first; // status code (dynamic) -> 200
	_htx["A"][2] = status->second; // status msg (dynamic) -> OK
	_htx["A"][2] += "\r\n";
}

void	header_handler::gen_date()
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

void	header_handler::gen_serv() /* PROBLEM : S'IL N'Y A PAS DE CHAMP Server DS LA REQUETE TU SEGV PAR EX POUR UN POST DS GD TAILLE*/
{
	if (_htx["Server"].size() != 3)
		_htx["Server"].resize(3, string());
	_htx["Server"][0] = "Server: ";// HEADER_LABEL
	_htx["Server"][1] = _hrx["Host:"][0];// IP & HOST
	_htx["Server"][2] = "\r\n";

}

void	header_handler::gen_CType() /* PROBLEM : mieux vaudrait extraire ça d'un fichier et le récup ici (serait plus élégant)*/
{
// Capture file.ext(ension)
	string ext = _hrx["A"][1].substr(_hrx["A"][1].find_last_of(".") + 1);
#ifdef _debug_
	cout << "file ext asked : " << ext << endl;
#endif
// Content type : https://developer.mozilla.org/fr/docs/Web/HTTP/Basics_of_HTTP/MIME_types
	if ( ext == "/" || ext == "html" ) // Default file PROBLEM ?
		_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
	else if( ext == "ico" || ext == "png" || ext == "jpeg" || ext == "webp" || ext == "gif" || ext == "bmp" )
		_htx["Content-Type"].push_back("Content-Type: image\r\n");
	else if( ext == "ogg" || ext == "wav" || ext == "midi" || ext == "mpeg" || ext == "webm" )
		_htx["Content-Type"].push_back("Content-Type: audio\r\n");
	else if( ext == "ogg" || ext == "mp4" || ext == "webm" )
		_htx["Content-Type"].push_back("Content-Type: video\r\n");
		// _htx["Content-Type"].push_back("Content-Type: application/octet-stream\r\n");
}

void	header_handler::gen_CLength() /* PROBLEM : C'EST UN FOURRE TOUT QUI N'EST PAS BIEN CONÇU*/
{

/* 	string file("./files");
	file.append(_hrx["A"][1] == "/" ? "/index.html" : _hrx["A"][1]);
	ifstream fs(file.c_str(), std::ifstream::binary | std::ifstream::ate);
	if (!fs.is_open()) { // SI LE FICHIER N'EST PAS TROUVÉ ALORS ON ENVOIE LES PAGES D'ERREUR
		gen_startLine( _status.find("404") );
		fs.open(_si[_s_id].error_page,  std::ifstream::binary | std::ifstream::ate);
		if (!fs.is_open())
			throw (std::runtime_error( "Unkown file (header_writer) : error_4xx.html"));
		_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
	} */

/* PROBLEME */ // http://127.0.0.1:8080/test les gif 404 s'affiche http://127.0.0.1:8080/test/ <- '/' non

	ifstream fs;
	verify_file_openess(fs);
// GÉNÉRATION DU FIELD CONTENT-LENGTH
	stringstream ss;
	ss << fs.tellg();
	_htx["Content-Length"].push_back("Content-Length: "); // HEADER_LABEL
	_htx["Content-Length"].push_back( ss.str());
	_htx["Content-Length"].push_back( "\r\n");

// AJOUT DE TOUS LES FIELD A LA RESPONSE
	_response.clear();
	for (std::map<string, vector<string> >::iterator it = _htx.begin(); it != _htx.end(); it++)
		for (size_t i = 0, j = it->second.size(); i < j; ++i)
			_response += it->second[i];
	_response += "\r\n";
	cout << RED "Response :\n" RESET << _response << endl;

// AJOUT DU FICHIER À LA RESPONSE
	if (_hrx["A"][0] == "GET") { // S'IL S'AGIT D'UN GET ON JOINS LE FICHIER
		cout << RED "File written !" RESET  << endl;
		fs.seekg(ios_base::beg);
		_response.append((istreambuf_iterator<char>(fs)),
						 (istreambuf_iterator<char>() ));
	}
}

	/* FUNCTION SECONDAIRE : UTILITAIRES */

// reconnait quel server_virtuel va traiter la requete et initialise _s_id
void header_handler::set_server_id(void)
{
	size_t colon_pos = _hrx["Host:"][0].find_first_of(":");
	string host(_hrx["Host:"][0].substr(0,colon_pos));
	string port(_hrx["Host:"][0].substr(colon_pos +1));
	for (size_t i = _si.size(); i ; --i) {
		if ( _si[i - 1].host == host && _si[i - 1].port == port ) {
			_s_id = i - 1;
// SI LE SERVER_NAME EST == ON BREAK SINON ON CONTINUE JUSQU'AU PREMIER SERVER /* PROBLEM ? */
			for (size_t j = 0; i < _si[i - 1].server_name.size(); ++j)
				if ( _si[i - 1].server_name[j] == host )
					break ;
#ifdef _debug_
			cout << "host : " << host << ", port : " << port << ", id : " << _s_id << endl;
#endif
		}
	}
}

void header_handler::handle_get_rqst(void)
{
	gen_CType();
	gen_CLength(); // Add ContentLength and Body
}

void header_handler::handle_post_rqst(void) 
{
	if (_hrx.find("Content-Type:") != _hrx.end())
		cout << "_hrx['Content-Type:'].size() : " << _hrx["Content-Type:"].size() << " :" << endl;
	for (auto i = _hrx["Content-Type:"].begin(); i != _hrx["Content-Type:"].end(); i++)
		cout << "[" << *i << "]" << endl;
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
void	header_handler::verify_file_openess(ifstream& fs)
{
	string path;
	resolve_path(path);

	// path.append( _hrx["A"][1] == "/" ? "/index.html" : _hrx["A"][1] );
	fs.open(path.c_str(), std::ifstream::binary | std::ifstream::ate);
	if (!fs.is_open()) { // SI LE FICHIER N'EST PAS TROUVÉ ALORS ON ENVOIE LES PAGES D'ERREUR
		gen_startLine( _status.find("404") );
		if (_si[_s_id].error_page.empty()) // CHOISI LE FICHIER D'ERREUR PAR DEFAULT OU CELUI DE LA CONF
			fs.open("files/error_pages/error_4xx.html",  std::ifstream::binary | std::ifstream::ate);
		else
			fs.open(_si[_s_id].error_page,  std::ifstream::binary | std::ifstream::ate);
		if (!fs.is_open())
			throw (std::runtime_error( "Unkown path (header_writer) : error_4xx.html"));
		if (_htx["Content-Type"].empty())
			_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
		else
			_htx["Content-Type"][0] = "Content-Type: text/html; charset=utf-8\r\n";
	}
}


// Permet de séléctionner la location qui partage le plus avec l'url (comme le fait nginx)
void	header_handler::resolve_path(string& path)
{
// REMOVE TRAILING '/' AT URL'S END
	while (_hrx["A"][1].back() == '/' && _hrx["A"][1].size() != 1)
		_hrx["A"][1].pop_back();

	size_t pos;
	string url, uri;
	if (_hrx["A"][1] == "/") {
		cout << "ds if\n";
		path =  "." + _si[_s_id].location[0].location + "/" + _si[_s_id].location[0].root + "/" + _si[_s_id].location[0].index;
	}
	else if ((pos = _hrx["A"][1].find("/", 1, 1)) == string::npos) {
		cout << "ds else if\n";
		path = "." + _si[_s_id].location[0].location + _si[_s_id].location[0].root + _hrx["A"][1];
	}
	else { // SPLIT URL (path...) AND URI POUR RECHERCHE DS LES "LOCATION" DU SERVER CONCERNÉ
		cout << "ds else\n";
		url = _hrx["A"][1].substr(0, pos);
		uri = _hrx["A"][1].substr(pos);
		cout << BLUE "url : " RESET << url << BLUE " uri : " RESET << uri << endl;
		for ( size_t i = 0, len = 0; i < _si[_s_id].location.size() ; ++i )
			if ( _si[_s_id].location[i].location.find(url) == 0 && _si[_s_id].location[i].location.size() > len ) {
				len = _si[_s_id].location[i].location.size();
				path = "." + _si[_s_id].location[0].location + (_si[_s_id].location[i].root.back() == '/' ? _si[_s_id].location[i].root : _si[_s_id].location[i].root + "/");
				path += uri;//(_si[_s_id].location[i].location.back() == '/' ? _si[_s_id].location[i].location : _si[_s_id].location[i].location + "/");
				cout << BLUE "should never come out unless url and location share path : " RESET << path << " (== location + uri)" << endl;
				// if (len == )
					break ;
			}
	}
	// SI ON EST DIRECTEMENT SUR L'URI
/* 	else if (pos == 0) {
		cout << "ds else if\n";
		path = _si[_s_id].location[0].root + (_hrx["A"][1] == "/" ? _si[_s_id].location[0].index : _hrx["A"][1]);
	}
	else {
		cout << "ds else\n";
		url = _hrx["A"][1];
	} */

	cout << BLUE "url : " RESET << url << BLUE ", uri : " RESET << uri << BLUE ", path : " RESET << path << endl;

	if (path.empty()) {
		path = "." + (_si[_s_id].location[0].location.back() == '/') ? _si[_s_id].location[0].location : _si[_s_id].location[0].location + "/";
		path += _si[_s_id].location[0].root;
		// path += url; // c'est ici que "/test" se mets ds le path
	}
	path += uri;

	cout << BLUE "url : " RESET << url << BLUE ", uri : " RESET << uri << BLUE ", path : " RESET << path << endl;

	struct stat sb = {0}; // à la place de : bzero(&sb, sizeof(sb));
	if (lstat(path.c_str(), &sb) == -1) {
		perror("lstat");
		// exit(EXIT_FAILURE);
	}
	switch (sb.st_mode & S_IFMT) {
		// case S_IFBLK:  printf("block device\n");            break;
		// case S_IFCHR:  printf("character device\n");        break;
		case S_IFDIR:  printf("directory\n");	path = "./files/if_folder.html";		break;
		// case S_IFIFO:  printf("FIFO/pipe\n");               break;
		// case S_IFLNK:  printf("symlink\n");                 break;
		case S_IFREG:  printf("regular file\n");            break;
		// case S_IFSOCK: printf("socket\n");                  break;
		default:
			printf("unknown? path : %s, uri : %s\n", path.c_str(), uri.c_str());
			if (lstat(path.c_str(), &sb) == -1)
				uri = "error_4xx.html";
			path = _si[_s_id].error_page.empty() ? "./files/error_pages/" + uri : _si[_s_id].error_page + uri;
			break;
	}

#ifdef _debug_
	cout << BLUE "resolved_path : " RESET << path << " uri(" + uri + ")" << endl;
#endif
}


	/* FUNCTION GETTER / SETTER */

std::string &header_handler::get_response(void) {return _response;}

	/* FUNCTION DE DEBUG */

void header_handler::display(void) {
	cout << GREEN ITALIC UNDERLINE "DISPLAY HEADER INFORMATION" RESET GREEN " :" RESET << endl;
	for (map<string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it) {
		cout << it->first << " ";
		for (size_t i = 0; i < it->second.size(); ++i)
			cout << it->second[i] << ", ";
		cout << endl;
	}
}

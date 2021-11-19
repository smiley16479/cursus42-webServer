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
	for (map< string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it)
		cout <<  YELLOW "map it.first : " RESET << it->first << " size() : " << it->second.size() << endl; */
const char *array[] = {	"A", "Accept:", "Accept-Language:", "Host:", "User-Agent:",
							"Accept-Encoding:", "Connection:", "Upgrade-Insecure-Requests:",
												"Cache-Control:", "DNT:", "rol:"};
	for (size_t i = 0; i < sizeof(array) / sizeof(const char*); ++i)
		_hrx[array[i]];
		// _hrx.insert(std::make_pair(array[i], vector<string>()));
	for (map< string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it)
		cout <<  YELLOW "map it.first : " RESET << it->first << " size() : " << it->second.size() << endl;

	ifstream fs("configuration_files/HTML_error_msg.txt");
	if (!fs.is_open()) 
		throw (std::runtime_error( "Unkown file : configuration_files/HTML_error_msg.txt"));
	
	string buf_1, buf_2;
	while (std::getline(fs, buf_1)) {
		if (buf_1[0] == '#' || buf_1[0] == '\0')
			continue ;
		std::stringstream ss_2(buf_1);
		ss_2 >> buf_1; 
		std::getline(ss_2, buf_2);
		_error[buf_1].append(buf_2.substr(0, buf_2.find_first_of('\t')));
	}
	for (map< string, string>::iterator it = _error.begin(), end = _error.end(); it != end; ++it)
	cout <<  YELLOW "map it.first : [" RESET << it->first << "] second : [" << it->second << "]" << endl;
//	exit(0);
}

header_handler::~header_handler()
{
}

/* cout << distance(mymap.begin(),mymap.find("198765432")); */ // <- Get index of the pair(key_type, mapped_type)

void header_handler::reader(char *str)
{
	string buf_1, buf_2;
	std::stringstream ss_1(str);
	cout << RED "DANS HEADER READER" RESET << str << endl;
	while (std::getline(ss_1, buf_1)) {
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
		cout << GREEN "ss_2 >> buf_2 : " << buf_2 << RESET << endl;
		cout << RED << buf_1 << RESET << endl;
		cout << MAGENTA << "tour" << RESET << endl;
#endif
	}
	// cout << RED "APRES GETLINE : " << buf_1 << RESET << endl;
	// this->display();
}

void header_handler::writer(void) {

	gen_startLine();
	gen_date();
	gen_serv();
	gen_CType();
	gen_CLength(); // Add ContentLength and Body

	_hrx.clear();
	_htx.clear();
}

void header_handler::display(void) {
	cout << GREEN ITALIC UNDERLINE "DISPLAY HEADER INFORMATION" RESET GREEN " :" RESET << endl;
	for (map<string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it) {
		cout << it->first << " ";
		for (size_t i = 0; i < it->second.size(); ++i)
			cout << it->second[i] << ", ";
		cout << endl;
	}
}

std::string &header_handler::get_response(void) {return _response;}


	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */

void	header_handler::gen_startLine() /* PROBLEM */
{
	_htx["A"].push_back("HTTP/1.1 "); // version (static)
	_htx["A"].push_back("200 "); // status (dynamic)
	_htx["A"].push_back("OK\r\n"); // status msg (dynamic)

	string file("./files");
	file.append(_hrx["GET"][0] == "/" ? "/index.html" : _hrx["GET"][0]);
	ifstream fs(file.c_str(), std::ifstream::binary | std::ifstream::ate);
	if (!fs.is_open()) {
		_htx["A"][1] = "404 ";
		_htx["A"][2] = "Not Found\r\n";
	}
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

void	header_handler::gen_serv() /* PROBLEM */
{
	//_response += "Server: ";// HEADER_LABEL
	//_response += _hrx["Host:"][0]; // IP & HOST
	// //_response += "0.0.0.0"; // IP
	// //_response += ":8080\r\n"; // PORT
	//_response += "\r\n";

	_htx["Server"].push_back("Server: ");// HEADER_LABEL
	_htx["Server"].push_back(_hrx["Host:"][0]);
	_htx["Server"].push_back("\r\n");

}

void	header_handler::gen_CType() /* PROBLEM */
{// IF YOU WANT THE BROWSER TO READ WITHOUT SKING TO DOWNLOAD IT, DON'T MENTION ITS FILE TYPE
// Capture file.ext(ension)
	string ext = _hrx["GET"][0].substr(_hrx["GET"][0].find_last_of(".") + 1);
#ifdef _debug_
	cout << "file ext asked : " << ext << endl;
#endif
// Content type : https://developer.mozilla.org/fr/docs/Web/HTTP/Basics_of_HTTP/MIME_types
	if ( ext == "/" || ext == "html" ) // Default file PROBLEM ?
		_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
	else if( ext == "ico" || ext == "png" || ext == "jpeg" || ext == "webp" || ext == "gif" || ext == "bmp" )
		_htx["Content-Type"].push_back("Content-Type: image/\r\n");
	else if( ext == "ogg" || ext == "wav" || ext == "midi" || ext == "mpeg" || ext == "webm" )
		_htx["Content-Type"].push_back("Content-Type: audio/\r\n");
	else if( ext == "ogg" || ext == "mp4" || ext == "webm" )
		_htx["Content-Type"].push_back("Content-Type: application/octet-stream\r\n");
}

void	header_handler::gen_CLength() /* PROBLEM */
{

	string file("./files");
	file.append(_hrx["GET"][0] == "/" ? "/index.html" : _hrx["GET"][0]);
	ifstream fs(file.c_str(), std::ifstream::binary | std::ifstream::ate);
	if (!fs.is_open()) {
		fs.open("./files/error_pages/error_4xx.html",  std::ifstream::binary | std::ifstream::ate);
		if (!fs.is_open())
			throw (std::runtime_error( "Unkown file (header_writer) : error_4xx.html"));
		_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
	}

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
	if (_hrx["GET"].size()) { // S'IL S'AGIT D'UN GET ON JOINS LE FICHIER
		cout << RED "File written !" RESET  << endl;
		fs.seekg(ios_base::beg);
		_response.append((istreambuf_iterator<char>(fs)),
						 (istreambuf_iterator<char>() ));
	}
}

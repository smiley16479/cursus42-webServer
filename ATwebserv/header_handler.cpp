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
		_hi[array[i]];
		// _hi.insert(std::make_pair(array[i], vector<string>()));
	for (map< string, vector<string> >::iterator it = _hi.begin(), end = _hi.end(); it != end; ++it)
		cout <<  YELLOW "map it.first : " RESET << it->first << " size() : " << it->second.size() << endl; */
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
			// if (_hi.find(buf_2) != _hi.end()) {
				string index = buf_2;
				while (ss_2 >> buf_2)
					_hi[index].push_back(buf_2);
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

string& header_handler::writer(void) {

	gen_startLine();
	gen_date();
	gen_serv();
	gen_CType();
	gen_CLenght();
	_hi.clear();
	return _response;
}

void header_handler::display(void) {
	cout << GREEN ITALIC UNDERLINE "DISPLAY HEADER INFORMATION" RESET GREEN " :" RESET << endl;
	for (map<string, vector<string> >::iterator it = _hi.begin(), end = _hi.end(); it != end; ++it) {
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
	_response = "HTTP/1.1 "; // version (static)
	_response += "200 "; // status (dynamic)
	_response += "OK\r\n"; // status msg (dynamic)
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

    _response.append(date);
}

void	header_handler::gen_serv() /* PROBLEM */
{
	_response += "Server: ";// HEADER_LABEL
	_response += _hi["Host:"][0];
	// _response += "0.0.0.0"; // IP
	// _response += ":8080\r\n"; // PORT
	_response += "\r\n";
}

void	header_handler::gen_CType() /* PROBLEM */
{// IF YOU WANT THE BROWSER TO READ WITHOUT SKING TO DOWNLOAD IT, DON'T MENTION ITS FILE TYPE
	_response += "Content-Type: "; // HEADER_LABEL

// Capture file.ext(ension)
	string ext = _hi["GET"][0].substr(_hi["GET"][0].find_last_of(".") + 1);
#ifdef _debug_
	cout << "file ext asked : " << ext << endl;
#endif
// Content type : https://developer.mozilla.org/fr/docs/Web/HTTP/Basics_of_HTTP/MIME_types
	if( ext == "/" || ext == "html" ) // Default file PROBLEM ?
		_response += "text/html; charset=utf-8";
	else if( ext == "ico" ||	ext == "png" ||	ext == "jpeg" || ext == "gif" || ext == "bmp" || ext == "webp" )
		_response += "image/";
	else if( ext == "ogg" || ext == "wav" || ext == "midi" || ext == "mpeg" || ext == "webm" )
		_response += "audio/";
	else if( ext == "ogg" || ext == "mp4" || ext == "webm" )
		_response += "application/octet-stream";
	_response += "\r\n";
}

void	header_handler::gen_CLenght() /* PROBLEM */
{
	_response += "Content-Lenght: "; // HEADER_LABEL

	string file("./files");
	file.append(_hi["GET"][0] == "/" ? "/index.html" : _hi["GET"][0]);
	cout << "file looked for : " "Unkown file (header_writer) : " + file << endl;
	ifstream fs(file.c_str(), std::ifstream::binary | std::ifstream::ate);
	if (!fs.is_open())
		// throw (std::runtime_error( "Unkown file (header_writer) : " + file));
		return;
	stringstream ss;
	
	ss << fs.tellg();
	_response.append(ss.str());
	_response += "\r\n\r\n";

	cout << RED "_response : " RESET << _response << endl;
	if (_hi["GET"].size()) { // S'IL S'AGIT D'UN GET ON JOINS LE FICHIER
		cout << RED "File written !" RESET  << endl;
		fs.seekg(ios_base::beg);
		_response.append((istreambuf_iterator<char>(fs)),
						 (istreambuf_iterator<char>() ));
	}
}
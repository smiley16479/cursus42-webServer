/* 
*      HTTP-message   = start-line
*                      *( header-field CRLF )
*                      CRLF
*                      [ message-body ]
*
*      start-line     = request-line (client)/ status-line (server)
*/

#include "header_handler.hpp"

header_handler::header_handler(/* args */)
{// BON EN FAIT IL SEMBLE QUE LA SECU NE SOIT PAS LE POINT IMPORTANT DE WEBSERV...
/* const char *array[] = {	"GET", "Host:", "User-Agent:", "Accept:", "Accept-Language:",
												"Accept-Encoding:", "Connection:", "Upgrade-Insecure-Requests:",
												"Cache-Control:", "nection:", "coding:", "DNT:", "rol:"};

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


//	// on devrait checker les mauvais mots comme '{''}' etc.
//	void extract_to_string(string& buf) {
//		string word;
//		std::stringstream ss(buf);
//		if (ss >> word) {
//			if (word.find(';', 0) == string::npos)
//				v = word;
//			else {
//				if (std::count(word.begin(), word.end(), ';') != 1 || word[word.size() -1] != ';')
//					throw (std::runtime_error( "bad formating(1) around : " + word));
//				word.resize(word.size() -1);
//				v = word;
//				return ; // On quite au cas ou la ';' est deja passee
//			}
//			if (ss >> word && word != ";")
//				throw (std::runtime_error( "bad formating(2) around : " + word));
//		}
//		else // au cas ou il n'y a plus de mots
//			throw (std::runtime_error( "bad formating(3) EOF reached"));
//	}
//	
//	void header_handler::header_checker(string& buf) {
//		std::stringstream ss(buf);
//		string word;
//		for (ss >> word; word != "}"; /* cout << BLUE "loca_tour de boucle word1 : " RESET << word << " bracket : " << bracket << endl, */ ss >> word) {
//			if (word == "GET")
//				extract_to_vector(buf);
//			else if (word == "Host:") {
//				extract_to_string(buf);
//				// std::cout << GREEN "auth_basic : " RESET << si.location[si.location.size() - 1].auth_basic << std::endl;
//			}
//			else if (word == "User-Agent:") {
//				extract_to_string(buf);
//				// std::cout << GREEN "auth_user_file : " RESET << si.location[si.location.size() - 1].auth_user_file << std::endl;
//			}
//			else if (word == "Accept:") {
//				extract_to_string(buf);
//				// std::cout << GREEN "autoindex : " RESET << si.location[si.location.size() - 1].autoindex << std::endl;
//			}
//			else if (word == "Accept-Language:") {
//				extract_to_string(buf);
//				// std::cout << GREEN "index : " RESET << si.location[si.location.size() - 1].index << std::endl;
//			}
//			else if (word == "Accept-Encoding:") {
//				extract_to_string(buf);
//				// std::cout << GREEN "max_file_size : " RESET << si.location[si.location.size() - 1].max_file_size << std::endl;
//			}
//			else if (word == "Connection:") {
//				extract_to_string(buf);
//				// std::cout << GREEN "return_directive : " RESET << si.location[si.location.size() - 1].return_directive << std::endl;
//			}
//			else if (word == "Upgrade-Insecure-Requests:") {
//				extract_to_vector(buf);
//				// std::cout << GREEN "root : " RESET << si.location[si.location.size() - 1].root << std::endl;
//			}
//			else
//				throw (std::runtime_error( "bad header format (header_checker) around : " + word));
//		}
//	}

/* 
* Extrait des arguments potentiellement multiples vers le vecteur de string de la directive
* Va foirrer si les arguments n'ont pas de ';' et mordent jusqu'à la fin de la seconde directive... (a réviser) :
* -> On peut ajouter un numero corecpondant au type directive et des test a faire en fonction pour y pallier
*/
/* void config_checker::extract_to_vector(std::string& str) { // les parametres ont été changés
	string word;
	ifs >> word;
	while (1)
	{
		if	(word.find(';', 0) == string::npos)
			v.push_back(word);
		else {
			if (std::count(word.begin(), word.end(), ';') != 1 || word[word.size() -1] != ';')
				throw (std::runtime_error( "bad formating (location_part vector) around : " + word));
			word.resize(word.size() -1);
			v.push_back(word);
			break ;
		}
		// std::cout << GREEN "allowed_method : " RESET << v.back() << std::endl;
		ifs >> word;
	}
	for (std::vector<string>::iterator it = v.begin(), end = v.end(); it != end; ++it)
		std::cout << GREEN "extract_to_Vector : " RESET << *it << std::endl;
} */

std::string &header_handler::get_response(void) {return _response;}


	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */

void	header_handler::gen_startLine() /* PROBLEM */
{
	_response = "HTTP/1.1 "; // version (static)
	_response += "200 "; // status (dynamic)
	_response += "OK\n"; // status msg (dynamic)
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
	_response += "0.0.0.0"; // IP
	_response += ":8080\n"; // PORT
}

void	header_handler::gen_CType() /* PROBLEM */
{
	_response += "Content-Type: "; // HEADER_LABEL
	_response += "image/gif\n"; // Content type : https://developer.mozilla.org/fr/docs/Web/HTTP/Basics_of_HTTP/MIME_types
}

void	header_handler::gen_CLenght() /* PROBLEM */
{
	_response += "Content-Lenght: "; // HEADER_LABEL

	string file("./files/dancing-banana.gif");
	ifstream fs(file.c_str(), std::ifstream::binary | std::ifstream::ate);
	if (!fs.is_open())
		throw (std::runtime_error( "Unkown file (header_writer) : " + file));
	stringstream ss;
	
	ss << fs.tellg();
	_response.append(ss.str());
	_response += "\n";
	_response += "\r\n";

	cout << RED "_response : " RESET << _response << endl;
	if (_hi["GET"].size()) { // S'IL S'AGIT D'UN GET ON JOINS LE FICHIER
		fs.seekg(ios_base::beg);
		_response.append((istreambuf_iterator<char>(fs)),
						 (istreambuf_iterator<char>() ));
	}
}
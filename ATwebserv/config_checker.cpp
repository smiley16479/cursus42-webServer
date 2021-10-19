#include "config_checker.hpp"
#include <string>
#include <iostream>
using namespace std;

config_checker::config_checker()
{
	std::ifstream ifs("./configuration_files/webserv_config.txt");// A remplace par av[1] en prod
	if (ifs.fail()) {
		std::cout << "webserv_config.txt does not exit\n"; // <-- IDEM
		return ;
	}
	string str;
	while (std::getline(ifs, str)) {
		if (str[0] == '#')
			continue ;
		#if debug
			std::cout << str << ", str.substr(0, str.find(' \t')) : " << str.substr(0, str.find_first_of(" \t")) << std::endl;
		#endif
		_semantic[str.substr(0, str.find_first_of(" \t"))];
	}
	for (map<string, vector <string>>::iterator it = _semantic.begin(), end = _semantic.end(); it != end; ++it)
		cout << it->first << endl;
	cout << "Fin du constructeur..." << endl;
}

config_checker::~config_checker()
{
}

// void config_checker::check_conFile(std::string str)
// {
// 	std::ifstream ifs(str.c_str());
// 	size_t pos;
// 	if (ifs.fail()) {
// 		std::cout << str << "does not exit\n";
// 		return ;
// 	}
// 	string str1, str2;
// 	bool found(false);
// 	int i(0), j;
// 	while (std::getline(ifs, str1)) { // cpp01/ex07
// 		if (str1[0] == '#')
// 			continue ;
// 		++i;
// 		str2 += str1;
// /* 		char *saveptr1, *saveptr2, *token, *subtoken;
// 		const char *strptr1, *strptr2;
// 		for (j = 1, strptr1 = str2.data(); ; j++, strptr1 = NULL) {
// 			token = strtok_r((char *)strptr1, " \t", &saveptr1);
// 			if (token == NULL)
// 				break;
// 			printf("%d: %s\n", j, token);
// 			for (str2 = token; ; str2 = NULL) {
// 				subtoken = strtok_r(str2, argv[3], &saveptr2);
// 				if (subtoken == NULL)
// 					break;
// 				printf(" --> %s\n", subtoken);
// 			}
// 		} */
// 		found = false;
// 		for (map<string, string>::iterator it = _semantic.begin(), end = _semantic.end(); it != end; ++it)
// 			if ((pos = str1.substr(0, str1.find_first_of(" \t")).find(it->first, 0)) != string::npos) {
// 				// insert second arg in map;
// 				found = true;
// 				break ;
// 			}
// 		#if debug
// 			cout << RED << str1.substr(0, str1.find_first_of(" \t")) << " : shouldn't reach this for a correct config_file\n" RESET;
// 		#endif
// 		if (!found) {
// 			stringstream ss; //utilisation de stringstream car to_string viole -std=c++98
// 			ss << "l." << i;
// 			ss >> str1;
// 			str.append(" ");
// 			str.append(str1);
// 			throw (configException(str));
// 		}
// 	}
// }

void config_checker::check_conFile(std::string str)
{
	std::ifstream ifs(str.c_str());
	std::string word;
	if (ifs.fail()) {
		std::cout << str << "does not exit\n";
		return ;
	}
	while (ifs >> word) {
		cout << word;
		if (word == "server") {
			check_serv_part(ifs);
		}
	}
}

/* 
**		PRIVATE FUNCTION (UTILITAIRES)
*/

bool config_checker::is_not_allowed(string key,string val)
{
	
}

void config_checker::allowed_port(std::ifstream& ifs)
{
	string word;
	if (!(ifs >> word) || is_not_allowed("port", word));
		throw (configException("bad port"));
	int port(atoi(word.c_str()));
	if (80 > port && port > 65535)
		configException("bad port");
	_si.port = word;
	std::cout << "port : " << _si.port << std::endl;
}

void config_checker::check_serv_part(std::ifstream& ifs){
	const char  *mandatory_assets[] = {"port",
			"host",
			"server_name",
			"error_page",
			"time_out",
			"cgi_file_types",
			"location"};
	string word;
	ifs >> word;
	if (word !=  "{")
		throw (configException(word));
	int bracket(1);
	for (ifs >> word; word != "}" && bracket; ifs >> word)
		if (word == "port")
			allowed_port(ifs);
		else if (word == "server_name" && ifs >> word) {
			_si.server_name[_si.server_name.size()];
			// std::cout << "server_name : " << _si.server_name[_si.server_name.size() - 1] << std::endl;
		}
		else if (word == "error_page" && ifs >> word) {
			_si.error_page = word;
			std::cout << "error_page : " << _si.error_page << std::endl;
		}
		// else if (word == "host") // Quingli utilise "host" -> pk ?
			// ;
		else if (word == "time_out" && ifs >> word) {
			_si.time_out = word;
			std::cout << "time_out : " << _si.time_out << std::endl;
		}
		else if (word == "cgi_file_types" && ifs >> word) {
			_si.cgi_file_types = word;
			std::cout << "cgi_file_types : " << _si.cgi_file_types << std::endl;
		}
		else if (word == "location" && ifs >> word) {
			// check_loca_part(ifs);  // ?
			std::cout << "location : " << _si.location << std::endl;
		}
		else if (word == "{" || word == "}")
			word == "{" ? ++bracket : --bracket;
		else
			configException(word);
/* 	bool found(true);
	for (map<string, string>::iterator it = _semantic.begin(), endit = _semantic.end(); it != endit && found; ++it)
		for (size_t i = 0, found = false; i < sizeof(mandatory_assets) / sizeof(char*); ++i)
			if (it->first == mandatory_assets[i]) {
				found = true;	
				break ;
			}
	if (!found)
		throw (configException()); */
}

void config_checker::check_loca_part(std::ifstream& ifs){
	const char  *mandatory_assets[] = {"alias",
		// "allowed_method",
		// "auth_basic",
		// "auth_user_file",
		// "autoindex",
		// "index",
		// "max_file_size",
		"root"};

}


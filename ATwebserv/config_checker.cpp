#include "config_checker.hpp"

config_checker::config_checker()
{
	std::ifstream ifs("./configuration_files/webserv_config.txt");// A remplace par av[1] en prod
	if (ifs.fail()) {
		std::cout << "webserv_config.txt does not exit\n"; // <-- IDEM
		return ;
	}
	string str;
	int i(0);
	while (std::getline(ifs, str)) {
		if (str[0] == '#')
			continue ;
		#if debug
			std::cout << str << ", str.substr(0, str.find(' \t')) : " << str.substr(0, str.find_first_of(" \t")) << std::endl;
		#endif
		_semantic[str.substr(0, str.find_first_of(" \t"))];
		stringstream ss;
		string word;
		ss << str;
		while (ss >> word) {
			cout << "test " << i << " : " << word << endl;
		}
		++i;
	}
	for (map<string, vector<string> >::iterator it = _semantic.begin(), end = _semantic.end(); it != end; ++it)
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
	if (ifs.fail())
		throw (configException("Unfound"));
	while (ifs >> word) {
		cout << "check_ConFile : " << word << endl;
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
	return false;
}

void config_checker::valid_port(std::ifstream& ifs)
{
	string word;
	if (!(ifs >> word) || word.find_first_not_of("0123456789") != string::npos)
		throw (configException("bad port"));
	int port(atoi(word.c_str()));
	if (65535 < port || port < 80)
		throw (configException("bad port"));
	_si.port = word;
	std::cout << "port : " << _si.port << std::endl;
}

void config_checker::valid_server_nm(std::ifstream& ifs)
{// C'est quoi un server_name valide ... ? juste un nom ou il faut ping ou quoi
	string word;
	if (!(ifs >> word) || word.find_first_not_of("0123456789") != string::npos)
		throw (configException("bad port"));
	int port(atoi(word.c_str()));
	if (65535 < port || port < 80)
		throw (configException("bad port"));
	_si.port = word;
	std::cout << "port : " << _si.port << std::endl;
}

void config_checker::valid_error_page(std::ifstream& ifs)
{
	string word;
	if (!(ifs >> word) || word.find_first_not_of("0123456789") != string::npos)
		throw (configException("bad port"));
	int port(atoi(word.c_str()));
	if (65535 < port || port < 80)
		throw (configException("bad port"));
	_si.port = word;
	std::cout << "port : " << _si.port << std::endl;
}

void config_checker::check_serv_part(std::ifstream& ifs){
	string word;
	ifs >> word;
	if (word !=  "{")
		throw (configException(word));
	int bracket(1);
	for (ifs >> word; word != "}" || bracket; ifs >> word)
		if (word == "port")
			valid_port(ifs);
		else if (word == "server_name" && ifs >> word) {
			_si.server_name[_si.server_name.size()];
			// std::cout << "server_name : " << _si.server_name[_si.server_name.size() - 1] << std::endl;
		}
		else if (word == "error_page" && ifs >> word) {
			valid_error_page(ifs);
			_si.error_page = word;
			std::cout << "error_page : " << _si.error_page << std::endl;
		}
		// else if (word == "host") // Quingli utilise "host" au lieu de server_name et port au lieu de listen -> pk ?
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
			// std::cout << "location : " << _si.location[_si.location.size()] << std::endl;
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


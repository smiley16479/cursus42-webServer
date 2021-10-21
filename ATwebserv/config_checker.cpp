#include "config_checker.hpp"

config_checker::config_checker()
{
	std::ifstream ifs("./configuration_files/webserv_config.txt");// A remplace par av[1] en prod
	if (ifs.fail()) {
		std::cout << "webserv_config.txt does not exit\n"; // <-- IDEM
		return ;
	}
// SERVAIT LORS DE LA CONFIGURATION DU CHECKER POUR CONNAITRE À L'AVANCE LES TOKENS
	// string str;
	// int i(0);
	// while (std::getline(ifs, str)) {
	// 	if (str[0] == '#')
	// 		continue ;
	// 	#if debug
	// 		std::cout << str << ", str.substr(0, str.find(' \t')) : " << str.substr(0, str.find_first_of(" \t")) << std::endl;
	// 	#endif
	// 	_semantic[str.substr(0, str.find_first_of(" \t"))];
	// 	stringstream ss;
	// 	string word;
	// 	ss << str;
	// 	while (ss >> word) {
	// 		cout << "test " << i << " : " << word << endl;
	// 	}
	// 	++i;
	// }
	// for (map<string, vector<string> >::iterator it = _semantic.begin(), end = _semantic.end(); it != end; ++it)
	// 	cout << it->first << endl;
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

void config_checker::string_vector_insert(ifstream& ifs, string &where_to_insert)
{

}

void config_checker::valid_port(std::ifstream& ifs)
{
	string word;
	if (!(ifs >> word) || word.find_first_not_of("0123456789;") != string::npos)
		throw (configException("bad port"));
	int port(atoi(word.c_str()));
	if (65535 < port || port < 80)
		throw (configException("bad port"));
	_si.port = word;
	std::cout << "port : " << _si.port << std::endl;
}

void config_checker::valid_server_nm(std::ifstream& ifs)
{// C'est quoi un server_name valide ... ? juste un nom ou il faut ping ou quoi
// ...juste un nom apparrement
}

void config_checker::valid_error_page(std::ifstream& ifs)
{
	string word;
	if (!(ifs >> word))
		throw (configException("error_page"));
	_si.error_page = word;
	std::cout << "valid_error_page : " << _si.error_page << std::endl;
}

void config_checker::check_serv_part(std::ifstream& ifs) {
	string word;
	ifs >> word;
	if (word !=  "{")
		throw (configException(word));
	int bracket(1);
	for (ifs >> word; word != "}" && bracket; /* cout <<  MAGENTA "serv_tour de boucle word : " RESET << word << ", bracket : " << bracket << endl , */ ifs >> word)
		if (word == "port")
			valid_port(ifs);
		else if (word == "server_name" && ifs >> word) {
			_si.server_name.push_back(word);
			std::cout << "server_name : " << _si.server_name[_si.server_name.size() - 1] << std::endl;
		}
		else if (word == "max_file_size" && ifs >> word) {
			_si.max_file_size = word;
			std::cout << "max_file_size : " << _si.max_file_size << std::endl;
		}
		else if (word == "error_page" && ifs >> word) {
			// valid_error_page(ifs);
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
			_si.cgi_file_types.push_back(word);
			std::cout << "cgi_file_types : " << _si.cgi_file_types[_si.cgi_file_types.size() - 1] << std::endl;
		}
		else if (word == "location") {
			check_loca_part(ifs);  // ?
		}
		else if (word == "{" || word == "}")
			word == "{" ? ++bracket : --bracket;
		else
			/* throw */ (configException("bad formating around : " + word));
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
	string word;
	ifs >> word;
	_si.location.push_back(locati_info());
	_si.location[_si.location.size() - 1].location = word;
	std::cout << RED "location : " RESET << _si.location[_si.location.size() - 1].location << std::endl;

	if (!(ifs >> word) || word !=  "{")
		throw (configException("bad formating (location_part1) around : " + word));
	int bracket(1);

	
	for (ifs >> word; word != "}"; /* cout << BLUE "loca_tour de boucle word1 : " RESET << word << " bracket : " << bracket << endl, */ ifs >> word) {
		if (word == "allowed_method")
			extract_to_vector(ifs, _si.location[_si.location.size() - 1].allowed_method);
		else if (word == "auth_basic") {
			// _si.location[_si.location.size() - 1].auth_basic = word;
			extract_to_string(ifs, _si.location[_si.location.size() - 1].auth_basic);
			std::cout << GREEN "auth_basic : " RESET << _si.location[_si.location.size() - 1].auth_basic << std::endl;
		}
		else if (word == "auth_user_file") {
			// _si.location[_si.location.size() - 1].auth_user_file = word;
			extract_to_string(ifs, _si.location[_si.location.size() - 1].auth_user_file);
			std::cout << GREEN "auth_user_file : " RESET << _si.location[_si.location.size() - 1].auth_user_file << std::endl;
		}
		else if (word == "autoindex") {
			// _si.location[_si.location.size() - 1].autoindex = word;
			extract_to_string(ifs, _si.location[_si.location.size() - 1].autoindex);
			std::cout << GREEN "autoindex : " RESET << _si.location[_si.location.size() - 1].autoindex << std::endl;
		}
		else if (word == "index") {
			// _si.location[_si.location.size() - 1].index = word;
			extract_to_string(ifs, _si.location[_si.location.size() - 1].index);
			std::cout << GREEN "index : " RESET << _si.location[_si.location.size() - 1].index << std::endl;
		}
		else if (word == "max_file_size") {
			// _si.location[_si.location.size() - 1].max_file_size = word;
			extract_to_string(ifs, _si.location[_si.location.size() - 1].max_file_size);

			std::cout << GREEN "max_file_size : " RESET << _si.location[_si.location.size() - 1].max_file_size << std::endl;
		}
		else if (word == "return_directive") {
			// _si.location[_si.location.size() - 1].return_directive = word;
			extract_to_string(ifs, _si.location[_si.location.size() - 1].return_directive);

			std::cout << GREEN "return_directive : " RESET << _si.location[_si.location.size() - 1].return_directive << std::endl;
		}
		else if (word == "return") 
			extract_to_vector(ifs, _si.location[_si.location.size() - 1].retour);
		else if (word == "root") {
			// _si.location[_si.location.size() - 1].root = word;
			extract_to_string(ifs, _si.location[_si.location.size() - 1].root);
			std::cout << GREEN "root : " RESET << _si.location[_si.location.size() - 1].root << std::endl;
		}
		else if (word == "{" || word == "}")
			word == "{" ? ++bracket : --bracket;
		else
			throw (configException("bad formating (location_part3) around : " + word));
	}
}


void config_checker::extract_to_string(std::ifstream& ifs, string& v) { // on devrait checker les mauvis mot comme '{''}' etc.
	string word;
	if (ifs >> word) {
		if (word.find(';', 0) == string::npos)
			v = word;
		else {
			if (std::count(word.begin(), word.end(), ';') != 1 || word[word.size() -1] != ';')
				throw (configException("bad formating (location_part word1) around : " + word));
			word.resize(word.size() -1);
			v = word;
			return ; // On quite au cas ou la ';' est deja passee
		}
		if (ifs >> word && word != ";")
			throw (configException("bad formating (location_part word3) around : " + word));
	}
	else // au cas ou il n'y a plus de mots
		throw (configException("bad formating (location_part word2) around : " + word));
}


void config_checker::extract_to_vector(std::ifstream& ifs, std::vector<string>& v) {
	string word;
	ifs >> word;
	while (1)
	{
		if	(word.find(';', 0) == string::npos)
			v.push_back(word);
		else {
			if (std::count(word.begin(), word.end(), ';') != 1 || word[word.size() -1] != ';')
				throw (configException("bad formating (location_part vector) around : " + word));
			word.resize(word.size() -1);
			v.push_back(word);
			break ;
		}
		// std::cout << GREEN "allowed_method : " RESET << v.back() << std::endl;
		ifs >> word;
	}
	for (std::vector<string>::iterator it = v.begin(), end = v.end(); it != end; ++it)
		std::cout << GREEN "extract_to_Vector : " RESET << *it << std::endl;
}
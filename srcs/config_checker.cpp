#include "config_checker.hpp"

config_checker::config_checker()
{
	#ifdef _debug_
		cout << RED "config_checker constructeur..." RESET << endl;
	#endif
}

config_checker::~config_checker()
{// appel du destructeur en code flow : confCheck.~config_checker(); 
	#ifdef _debug_
		cout << RED "config_checker destructeur..." RESET << endl;
	#endif
}

void config_checker::check_conFile(std::string str)
{
	std::ifstream ifs(str.c_str());
	std::string word;
	if (ifs.fail())
		throw (configException(_si, "Unfound"));
	while (ifs >> word) {
#ifdef _debug_
		cout << "check_ConFile : " << word << endl;
#endif
		if (word == "server") {
			_si.push_back(server_info());
			check_serv_part(ifs, _si[_si.size() - 1]);
		}
#ifdef _debug_
		cout << MAGENTA "check_ConFile : " RESET << _si.size() << endl;
#endif
	}
}


void config_checker::valid_port(std::ifstream& ifs, server_info& si)
{
	string word;
	if (!(ifs >> word) || word.find_first_not_of("0123456789;") != string::npos)
		throw (configException(_si, "bad port"));
	int port(atoi(word.c_str()));
	if (65535 < port || port < 80)
		throw (configException(_si, "bad port"));

	if (word.find(';', 0) == string::npos) {
		si.port = word;
		if (ifs >> word && word != ";")
			throw (configException(_si, "bad formating(2) around : " + word));
	}
	else if (std::count(word.begin(), word.end(), ';') != 1 || word[word.size() -1] != ';')
			throw (configException(_si, "bad formating(1) around : " + word));
	else {
		word.resize(word.size() -1);
		si.port = word;
	}
#ifdef _debug_
	std::cout << "port : " << si.port << std::endl;
#endif
}

void config_checker::check_serv_part(std::ifstream& ifs, server_info& si) {
	string word;
	ifs >> word;
	if (word !=  "{")
		throw (configException(_si, word));
	int bracket(1);
	for (ifs >> word; word != "}" && bracket; /* cout <<  MAGENTA "serv_tour de boucle word : " RESET << word << ", bracket : " << bracket << endl , */ ifs >> word)
		if (word == "port")
			valid_port(ifs, si);
		else if (word == "server_name" /* && ifs >> word */) {
			// si.server_name.push_back(word);
			std::cout << "server_name : ";
			extract_to_string(ifs, si.server_name);
			// std::cout << "server_name : " << si.server_name[si.server_name.size() - 1] << std::endl;
		}
		else if (word == "max_file_size") {
			extract_to_string(ifs, si.max_file_size);
			std::cout << "max_file_size : " << si.max_file_size << std::endl;
		}
		else if (word == "error_page") {
			extract_to_string(ifs, si.error_page);
			std::cout << "error_page : " << si.error_page << std::endl;
		}
		else if (word == "host") {// Quingli utilise "host" au lieu de server_name et port au lieu de listen -> pk ?
			extract_to_string(ifs, si.host);
			std::cout << "host : " << si.host << std::endl;
		}
		else if (word == "time_out") {
			extract_to_string(ifs, si.time_out);
			std::cout << "time_out : " << si.time_out << std::endl;
		}
		else if (word == "location") {
			check_loca_part(ifs, si);  // ?
		}
		else if (word == "{" || word == "}")
			word == "{" ? ++bracket : --bracket;
		else
			throw (configException(_si, "bad formating around : " + word));
}

void config_checker::check_loca_part(std::ifstream& ifs, server_info& si){

	string word;
	ifs >> word;
	si.location.push_back(locati_info());
	si.location[si.location.size() - 1].location = word;
	std::cout << RED "location : " RESET << si.location[si.location.size() - 1].location << std::endl;

	if (!(ifs >> word) || word !=  "{")
		throw (configException(_si, "bad formating (location_part1) around : " + word));
	int bracket(1);// reliquat d'un test qu'on peut virer pour le moment ici et sd le dernier else if
	
	for (ifs >> word; word != "}"; /* cout << BLUE "loca_tour de boucle word1 : " RESET << word << " bracket : " << bracket << endl, */ ifs >> word) {
		if (word == "allowed_method")
			extract_to_vector(ifs, si.location[si.location.size() - 1].allowed_method);
		else if (word == "auth_basic") {
			extract_to_string(ifs, si.location[si.location.size() - 1].auth_basic);
			std::cout << GREEN "auth_basic : " RESET << si.location[si.location.size() - 1].auth_basic << std::endl;
		}
		else if (word == "auth_user_file") {
			extract_to_string(ifs, si.location[si.location.size() - 1].auth_user_file);
			std::cout << GREEN "auth_user_file : " RESET << si.location[si.location.size() - 1].auth_user_file << std::endl;
		}
		else if (word == "autoindex") {
			extract_to_string(ifs, si.location[si.location.size() - 1].autoindex);
			std::cout << GREEN "autoindex : " RESET << si.location[si.location.size() - 1].autoindex << std::endl;
		}
		else if (word == "index") {
			extract_to_string(ifs, si.location[si.location.size() - 1].index);
			std::cout << GREEN "index : " RESET << si.location[si.location.size() - 1].index << std::endl;
		}
		else if (word == "max_file_size") {
			extract_to_string(ifs, si.location[si.location.size() - 1].max_file_size);
			std::cout << GREEN "max_file_size : " RESET << si.location[si.location.size() - 1].max_file_size << std::endl;
		}
		else if (word == "return_directive") {
			extract_to_string(ifs, si.location[si.location.size() - 1].return_directive);
			std::cout << GREEN "return_directive : " RESET << si.location[si.location.size() - 1].return_directive << std::endl;
		}
		else if (word == "return") 
			extract_to_vector(ifs, si.location[si.location.size() - 1].retour);
		else if (word == "root") {
			extract_to_string(ifs, si.location[si.location.size() - 1].root);
			std::cout << GREEN "root : " RESET << si.location[si.location.size() - 1].root << std::endl;
		}
		else if (word == "cgi_path") {
			extract_to_string(ifs, si.location[si.location.size() - 1].cgi_path);
			std::cout << "cgi_path : " << si.location[si.location.size() - 1].cgi_path << std::endl;
		}
		else if (word == "cgi_file_types" /* && ifs >> word */) {
			// si.cgi_file_types.push_back(word);
			std::cout << "cgi_file_types : ";
			extract_to_vector(ifs, si.location[si.location.size() - 1].cgi_file_types);
			// std::cout << "cgi_file_types : " << si.cgi_file_types[si.cgi_file_types.size() - 1] << std::endl;
		}
		else if (word == "upload_path") {
			extract_to_string(ifs, si.location[si.location.size() - 1].upload_path);
			std::cout << "upload_path : " << si.location[si.location.size() - 1].upload_path << std::endl;
		}
		else if (word == "{" || word == "}")
			word == "{" ? ++bracket : --bracket;
		else
			throw (configException(_si, "bad formating (location_part3) around : " + word));
	}
}

// on devrait checker les mauvais mots comme '{''}' etc.
void config_checker::extract_to_string(std::ifstream& ifs, string& v) {
	string word;
	if (ifs >> word) {
		if (word.find(';', 0) == string::npos)
			v = word;
		else {
			if (std::count(word.begin(), word.end(), ';') != 1 || word[word.size() -1] != ';')
				throw (configException(_si, "bad formating(1) above : " + word));
			word.resize(word.size() -1);
			v = word;
			return ; // On quite au cas ou la ';' est deja passee
		}
		if (ifs >> word && word != ";")
			throw (configException(_si, "bad formating(2) above : " + word));
	}
	else // au cas ou il n'y a plus de mots
		throw (configException(_si, "bad formating(3) EOF reached"));
}

/* 
* Extrait des arguments potentiellement multiples vers le vecteur de string de la directive
* Va foirrer si les arguments n'ont pas de ';' et mordent jusqu'à la fin de la seconde directive... (a réviser) :
* -> On peut ajouter un numero corecpondant au type directive et des test a faire en fonction pour y pallier
*/
void config_checker::extract_to_vector(std::ifstream& ifs, std::vector<string>& v) {
	string word;
	ifs >> word;
	while (1)
	{
		if	(word.find(';', 0) == string::npos)
			v.push_back(word);
		else {
			if (std::count(word.begin(), word.end(), ';') != 1 || word[word.size() -1] != ';')
				throw (configException(_si, "bad formating (location_part vector) around : " + word));
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

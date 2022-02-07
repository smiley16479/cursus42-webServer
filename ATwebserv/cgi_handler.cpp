#include "cgi_handler.hpp"
#include "request_handler.hpp"
#include "color.hpp"
using namespace std;


cgi_handler::cgi_handler(request_handler &obj) : obj(obj)
{
}



// Extrait la réponse des cgi vers la réponse à faire au client
int cgi_handler::cgi_output()
{
#ifdef _debug_
	cout << BLUE "IN CGI_OUTPUT()\n" RESET;
#endif
	char str[100000];
	int len, nb = 0;
	obj._body.clear();
	while ((len = read(obj._c->cgi_fd[0], str, 100000)) > 0) {// Peut-etre pas faire de while là
		obj._body.append(str, len);
#ifdef _debug_
		cout << GREEN "read len : " RESET "[" << len << "]" << " nb [" << ++nb << "]" << endl;
#endif
	}
#ifdef _debug_
	if (_body.size() < 1000)
		cout << GREEN "DS CGI_OUTPUT _body : \n" RESET "[" << _body + "]" << endl;
	else 
		cout << GREEN "DS CGI_OUTPUT _body(TOO_LONG) : \n" RESET "[" << _body.size() << "]" << endl;
#endif
	if (len < 100000) {
		std::cout << "CGI EOF" << std::endl;
		waitpid(-1, NULL, 0);
		clean_body();
// ECRIT EFFECTIVEMENT LA RÉPONSE COMPLETE (AV LES HEADERS)
		obj.gen_startLine( obj._status.find("200") );
		obj.gen_CLength(3);
		obj.add_all_field(); 
		obj.add_body(true);
		obj._hrx.clear();
		obj._htx.clear();
#ifdef _log_
	ofstream _LOGfile;
	_LOGfile.open("POST.txt", std::ofstream::trunc);
	if (_LOGfile.is_open()) {
		_LOGfile << obj._c->resp;
		_LOGfile.close();
	}
#endif
	}
	return 0;
}

  std::string to_upper(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
  }


/* VERSION SOLAL */

char** cgi_handler::setCGIEnv()
{
#ifdef _debug_
	cout << BLUE "IN SETCGIENV() : " RESET << (int)_c->rqst_t << "\n";
#endif

	std::map< std::string, std::string > cgi_env_;

  if (obj._c->rqst_t == POST){
		cgi_env_["CONTENT_TYPE"] = obj._hrx["Content-Type:"].empty() ? "" : obj._hrx["Content-Type:"][0];// req_headers_["Content-Type"];
		cgi_env_["CONTENT_LENGTH"] = obj._hrx["Content-Length:"].empty() ? std::to_string(obj._body.size()) : obj._hrx["Content-Length:"][0] ; // ft::to_string(req_body_.length());
	}
	cgi_env_["GATEWAY_INTERFACE"] = "CGI/1.1";
  cgi_env_["PATH_INFO"] = obj._path; //"ATwebserv/YoupirBanane"; // file_path_;
	cgi_env_["PATH_TRANSLATED"] = obj._path; // "ATwebserv/YoupirBanane"; // file_path_;
  cgi_env_["QUERY_STRING"] = obj._URLquery;//config_.getQuery();
  cgi_env_["REMOTE_ADDR"] = "127.0.0.1"; //config_.getClient().getAddr();

/* 
//   if (config_.getAuth() != "off") {
  if (_hrx.count("X-Secret-Header-For-Test:")) {
	cgi_env_["AUTH_TYPE"] = "Basic";
	// cgi_env_["REMOTE_IDENT"] = config_.getAuth().substr(0, config_.getAuth().find(':'));
	cgi_env_["REMOTE_IDENT"] = "test";
	// cgi_env_["REMOTE_USER"] = config_.getAuth().substr(0, config_.getAuth().find(':'));
	cgi_env_["REMOTE_USER"] = "test";

  }
 */

  cgi_env_["REQUEST_METHOD"] = obj._hrx["A"][0]; //config_.getMethod();
	cgi_env_["REQUEST_URI"] = obj._path; // file_path_;

  cgi_env_["SCRIPT_NAME"] = obj._c->loc->cgi_path; // cgi_path_;
	cgi_env_["SERVER_NAME"] = "WEBSERV/1.0"; //config_.getHost();
	cgi_env_["SERVER_PROTOCOL"] = "HTTP/1.1"; // config_.getProtocol();
	cgi_env_["SERVER_PORT"] = "8080"; // ft::to_string(config_.getPort());
  cgi_env_["SERVER_SOFTWARE"] = "WEBSERV/1.0";

//	if (extension_ == ".php")
//		cgi_env_["REDIRECT_STATUS"] = "200";

  for (std::map<std::string, std::vector <std::string> >::iterator it = obj._hrx.begin(); it != obj._hrx.end(); it++) {
	if (!it->second.empty()) 
		if (it->first == "X-Secret-Header-For-Test:")	{
	  std::string header = "HTTP_" + to_upper(it->first);
	  header.pop_back();
	  std::replace(header.begin(), header.end(), '-', '_');
	  cgi_env_[header] = it->second[0];
	}
  }
		char **env_;
	if (!(env_ = (char **)malloc(sizeof(char *) * (cgi_env_.size() + 1))))
		return NULL;

	int i = 0;

	for (std::map<std::string, std::string>::iterator it = cgi_env_.begin(); it != cgi_env_.end(); it++) {
		std::string tmp = it->first + "=" + it->second;
		if (!(env_[i] = strdup(tmp.c_str())))
					return NULL;
#ifdef _debug_
		else
			std::cout << "env : " << env_[i] << std::endl;
#endif
		i++;
	}
	env_[i] = NULL;
	return env_;
}


// Fait trop de truc...
int	cgi_handler::handle_cgi(void)
{
#ifdef _debug_
	cout << BLUE "IN HANDLE_CGI()\n" RESET;
#endif
	
	if (obj._c->loc->cgi_path.empty())
	{
#ifdef _debug_
		cout << YELLOW "cgi_path.empty\n" RESET;
#endif
		obj.gen_startLine( obj._status.find("403") );
		return 1;
	}
		// SI TOUT S'EST BIEN PASSÉ ON DÉTACHE LE BODY DE LA REQUETE POST (DS _BODY)
	if (obj._c->rqst_transfer_t == MULTIPART)
		obj.extract_multi_rqst_body();
	else if (obj._c->rqst_transfer_t == URL_ENCODED) // Content-Type: application/x-www-form-urlencoded
		obj.extract_xform_rqst_body();
	else if (obj._c->rqst_transfer_t == CHUNCK)
		obj.extract_chunk_rqst_body();


	if (launch_cgi()){

		if (cgi_input()) // Ne peut pas renvoyer autre chose que 0 pour le moment
			return 1;

#ifdef _debug_
	cout << YELLOW "3\n" RESET;
	if (_body.size() < 1000)
		std::cout << "Buffer copied in body :\n[" << _body + "]\n";
	else
		std::cout << "Buffer copied in body(TOO_LONG) :[" <<  _body.size() << "]\n";
#endif
		cgi_output();
	}
	else 
		return 1;

	return (0);
}

// Détecte s'il s'agit d'un cgi
bool	cgi_handler::is_cgi()
{
#ifdef _debug_
	cout << BLUE "IN IS_CGI() : " RESET;
#endif

	size_t	i;
	string query; // SI l'url comporte des argument après '?' on la tronque
	query = ((i = obj._path.find('?')) != string::npos) ? obj._path.substr(0, i) : obj._path;
	if (i != string::npos) {
		obj._URLquery = obj._path.substr(i + 1);
		obj._path = obj._path.substr(0 , i);
#ifdef _debug_
		cout << GREEN "_path : " RESET << _path << GREEN "_URLquery : " RESET<< _URLquery << "\n" ;
#endif
	}
	query = ((i = query.find_last_of('.')) != string::npos) ? query.substr(i) : query;
	i = 0;
	std::vector<std::string>& extensions = obj._c->loc-> cgi_file_types;
	for (std::vector<std::string>::iterator type = extensions.begin(); type != extensions.end(); type++, ++i)
	{
		if ((query.find(*type)) != std::string::npos){
#ifdef _debug_
			cout << "cgi type -> " + *type << endl;
#endif
			obj._c->ext_id = i;
			return (true);
		}
	}
#ifdef _debug_
	cout << "nop_not_cgi" << endl;
#endif
	return (false);
}

// Lance le fork() cgi avec le _path du fichier, ouvre les _c->cgi_fd[2];
int	cgi_handler::launch_cgi()
{
#ifdef _debug_
	cout << BLUE "IN LAUNCH_CGI()\n" RESET;
	cout << "path : " << _path << endl;
	cout << "cgi_program : " << _si[_s_id].location[_l_id].cgi_path.c_str() << endl;
#endif

	pid_t		pid;
	int			input_fd[2];	// pipe du body des requetes vers l'executable cgi
	int			output_fd[2];	// pipe de l'executable cgi vers les réponses des requetes
	char		*e_path[4] = {	(char*)obj._c->loc->cgi_path.c_str(),
								(char*)obj._path.c_str(),
								// (char*)"-c",
								// (char*)"files/scripts/php.ini",
								NULL
							 };

	if (pipe(input_fd) == -1 || pipe(output_fd) == -1)
		throw (std::runtime_error("Pipe initialization failled"));
	// fcntl(input_fd[0], F_SETFL, O_NONBLOCK); // <- A REMMETTRE ?
	// fcntl(input_fd[1], F_SETFL, O_NONBLOCK); // <- A REMMETTRE ?
	// fcntl(output_fd[0], F_SETFL, O_NONBLOCK); // <- A REMMETTRE ?

	// std::ifstream t(_path);
	// if (!t.is_open()) {
	// 	gen_startLine( _status.find("404") );
	// 	cout << RED "PAS OPEN!!\n";
	// 	return 0;
	// }

	pid = fork();
	if (pid == -1)
		throw (std::runtime_error("fork initialization failled"));
	if (pid == 0)
	{ // DS L'ENFANT
#ifdef _debug_
		cout << GREEN "\nCGI_FORK OK !\n" RESET;
#endif

		char**c_env = setCGIEnv();

		close(input_fd[1]);
		close(output_fd[0]);
		dup2(input_fd[0], STDIN_FILENO);	 // read
		dup2(output_fd[1], STDOUT_FILENO);	// write

		execve(*e_path, e_path, c_env);
		exit(1);
	}
	else // DS LE PARENT (on close les extrémités des pipes non utilisés)
	{
		close(input_fd[0]); // read
		close(output_fd[1]); // write
	}
	obj._c->cgi_fd[0] = output_fd[0];// ici on lit depuis l'executable des cgi
	obj._c->cgi_fd[1] = input_fd[1]; // ici on ecrit vers l'executable des cgi
	return (1);
}

// On ne sait pas ce que ça fait
void	cgi_handler::clean_body()
{
#ifdef _debug_
	cout << BLUE "IN CLEAN_BODY()\n" RESET;
#endif

	std::string	tmp;
	std::string	index;
	size_t	pos;
	
	while ((pos = obj._body.find("\r\n")) != std::string::npos)
	{
		tmp = obj._body.substr(0, pos + 2);
		obj._body = obj._body.substr(pos + 2);
		if (tmp == "\r\n")
			break ;
		if ((pos = tmp.find(":")) != std::string::npos)
		{
			index = tmp.substr(0, pos);
#ifdef _debug_
			cout << GREEN "IN CLEAN_BODY index=" RESET << index << endl;
#endif
			if (index != "X-Powered-By")
				obj._htx[index].push_back(tmp.substr(0, tmp.find("\r\n") + 2));
		}
	}
}

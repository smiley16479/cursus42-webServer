#include "cgi_handler.hpp"
#include "color.hpp"

void debug_mp_out(std::map<std::string, std::vector<std::string> >& mp)
{
	for (std::map<std::string, std::vector<std::string> >::iterator it = mp.begin(); it != mp.end(); it++)
	{
		for (std::vector<std::string>::iterator it1 = it->second.begin(); it1 != it->second.end(); it1++)
			std::cout << it->first << "=" << *it1 << std::endl;
	}
}

bool	is_cgi(std::vector<std::string>& query)
{
	for (std::vector<std::string>::iterator it = query.begin(); it != query.end(); it++)
	{
		if ((it->find(".php")) != std::string::npos)
			return (true);
	}
	return (false);
}

void	go_cgi(std::map<std::string, std::vector<std::string> >& mp, const server_info& serv)
{
	cgi_handler	cgi(mp, serv);
//	cgi_handler	cgi(av);
	std::string	tmp;
	int			pos;
	int			ret;
	int			fd[2];
	int			bfd[2];
	pid_t		pid;
	char		**plop = new char*[2];
	char		**lol = new char*[cgi.envSize() + 1];//{ (char*)"files/cgi/php-cgi", NULL };
	int	i;
//	std::string		out;

	plop[0] = (char*)CGI;
	plop[1] = NULL;
//	plop[1] = (char*)CGI_MODE;
//	plop[2] = NULL;
	i = 0;
	for (std::vector<std::string>::iterator it = cgi.envBegin(); it != cgi.envEnd(); it++, i++)
	{
		std::cout << GREEN << it->c_str() << RESET << std::endl;
		lol[i] = (char*)it->c_str();
	}
	lol[i] = NULL;
	bfd[0] = dup(STDIN_FILENO);
//	bfd[1] = dup(STDOUT_FILENO);
	if (pipe(fd) == -1)
		return ;
//		return (CRASH_PIPE);
	pid = fork();
	if (pid == -1)
		return ;
//		return (CRASH_FORK);
	if (pid == 0)
	{
		std::cout << "coucou" << std::endl;
		dup2(bfd[0], STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		execve(plop[0], NULL, lol);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		exit(1);
	}
	else
	{
		if (dup2(fd[0], STDIN_FILENO) == -1)
			return ;
		close(fd[1]);
		close(fd[0]);
		waitpid(pid, &ret, 0);
		/*
		if (WIFEXITED(ret))
		{
			if (WEXITSTATUS(ret) == 1)
				std::cout << "bad" << std::endl;
			else
				std::cout << "good" << std::endl;
		}
		else if (WIFSIGNALED(ret))
			std::cout << "bad" << std::endl;
		*/
		while (getline(std::cin, tmp))
		{
			if ((pos = tmp.find("Content-type: ")) != std::string::npos)
			{
				tmp = tmp.replace(0, strlen("Content-type: "), "");
				mp["Content-Type:"].clear();
				mp["Content-Type:"].push_back(tmp);
			}
			else if (!((pos = tmp.find("X-Powered-By:")) != std::string::npos))
			{
				if (mp["BODY"].empty())
					mp["BODY"] = std::vector<std::string>();
				mp["BODY"].push_back(tmp);
			}
		}
		std::cout << "coucou" << std::endl;
		dup2(bfd[0], STDIN_FILENO);
//		dup2(bfd[1], STDOUT_FILENO);
//		close(bfd[0]);
//		close(bfd[1]);
	}
	mp["A"].clear();
	/*
	mp["A"].erase(mp["A"].begin());
	mp["A"].erase(mp["A"].begin());
	mp["A"].push_back(" 200");
	mp["A"].push_back(" OK\r\n");
	*/
	delete [] lol;
}

cgi_handler::cgi_handler()	{
}

cgi_handler::cgi_handler(std::map<std::string, std::vector<std::string> >& mp, const server_info& serv)	{
	extract_env(mp, serv);
}

cgi_handler::cgi_handler(const cgi_handler& other)	{
	handler = (char*)CGI;
	env = other.env;
}

cgi_handler::~cgi_handler()	{
	handler.clear();
	env.clear();
}

cgi_handler&	cgi_handler::operator=(const cgi_handler& other)	{
	return (*this);
}

void	cgi_handler::extract_env(std::map<std::string, std::vector<std::string> >& mp,const server_info& _s)
{
	std::string			tmp;
	std::string			buf(mp["A"][1]);
	std::string			var;
	size_t				pos;

	tmp =  "REDIRECT_STATUS=CGI";
	env.push_back(tmp);
	tmp = "SERVER_SOFTWARE=";
	for (size_t j = 0; j < _s.server_name.size(); j++)
		tmp += _s.server_name[j];
	env.push_back(tmp);
	tmp = "SERVER_NAME=";
	tmp += _s.host;
	env.push_back(tmp);
	tmp = "GATEWAY_INTERFACE=CGI/1.1";
	env.push_back(tmp);
	tmp = "SERVER_PROTOCOL=HTTP/1.1";
	env.push_back(tmp);
	tmp = "SERVER_PORT=";
	tmp += _s.port;
	env.push_back(tmp);
	tmp = "REQUEST_METHOD=";
	tmp += mp["A"][0];
	env.push_back(tmp);
	tmp = "PATH_INFO=";
	pos = buf.find(".php");
	var = buf.substr(pos + 4);
	buf = buf.substr(0, pos + 4);
	if ((pos = var.find("/")) != std::string::npos)
		tmp += var.substr(pos + 1);
	env.push_back(tmp);
	tmp = "PATH_TRANSLATED=";
	tmp += _s.location[0].root;
	tmp += (buf[0] == '/' ? buf.substr(1) : buf);
	env.push_back(tmp);
	tmp = "SCRIPT_NAME=";
	tmp += _s.location[0].root;
	tmp += (buf[0] == '/' ? buf.substr(1) : buf);
//	tmp += buf.substr(buf.find_last_of("/") + 1);
	env.push_back(tmp);
	tmp = "QUERY_STRING=";
	if ((pos = var.find("?")) != std::string::npos)
		tmp += var.substr(pos + 1);
		/*
	if (!mp["BODY"].empty())
	{
		for (std::vector<std::string>::iterator it = mp["BODY"].begin(); it != mp["BODY"].end(); it++)
			tmp += *it;
		mp["BODY"].clear();
	}
	*/
	env.push_back(tmp);
	tmp = "REMOTE_HOST=";
	if (!mp["Host:"].empty())
	{
		for (size_t j = 0; j < mp["Host:"].size(); j++)
			tmp+= mp["Host:"][j];
	}
	/*
	env.push_back(tmp);
	tmp = "AUTH_TYPE=";
	if (!mp["Authorization:"].empty())
	{
		tmp+= mp["Authorization:"][0];
	}
	env.push_back(tmp);
	tmp = "REMOTE_USER=";
	// GET USER NAME FROM SERVER
	env.push_back(tmp);
	tmp = "REMOTE_IDENT=";
	// GET USER ID FROM SERVER
	env.push_back(tmp);
	tmp = "CONTENT_TYPE=";
	if (!mp["Content-type:"].empty())
	{
		for (size_t j = 0; j < mp["Content-type:"].size(); j++)
			tmp+= mp["Content-type:"][j];
	}
	env.push_back(tmp);
	tmp = "CONTENT_LENGTH=";
	if (!mp["Content-length:"].empty())
	{
		for (size_t j = 0; j < mp["Content-length:"].size(); j++)
			tmp+= mp["Content-length:"][j];
	}
	env.push_back(tmp);
	*/
	tmp = "HTTP_ACCEPT=";
	if (!mp["Accept:"].empty())
	{
		for (size_t j = 0; j < mp["Accept:"].size(); j++)
			tmp+= mp["Accept:"][j];
	}
	env.push_back(tmp);
	tmp = "HTTP_ACCEPT_LANGUAGE=";
	if (!mp["Accept-Language:"].empty())
	{
		for (size_t j = 0; j < mp["Accept-Language:"].size(); j++)
			tmp+= mp["Accept-Language:"][j];
	}
	env.push_back(tmp);
	tmp = "HTTP_USER_AGENT=";
	if (!mp["User-Agent:"].empty())
	{
		for (size_t j = 0; j < mp["User-Agent:"].size(); j++)
			tmp+= mp["User-Agent:"][j];
	}
	env.push_back(tmp);
//	tmp = "HTTP_COOKIE=";
	//GET COOKIE FROM SERVER
	//C EST UN BONUS !!!
//	env.push_back(tmp);
	/*
	tmp = "HTTP_REFERER=";
	if (!mp["Referer"].empty())
	{
		for (size_t j = 0; j < mp["Referer"].size(); j++)
			tmp+= mp["Referer"][j];
	}
	env.push_back(tmp);
		cout << "auth_basic : " << _s.location[j].auth_basic << endl;
		cout << "auth_user_file : " << _s.location[j].auth_user_file << endl;
		cout << "autoindex : " << _s.location[j].autoindex << endl;
		cout << "return_directive : " << _s.location[j].return_directive << endl;
		cout << "allowed_method : ";
		for (size_t k = 0; k < _s.location[j].allowed_method.size(); k++)
			cout << _s.location[j].allowed_method[k] << (k < _s[i].location[j].allowed_method.size() - 1 ? ", " : "");
		cout << endl << "return : ";
		for (size_t k = 0; k < _s.location[j].retour.size(); k++)
			cout << _s.location[j].retour[k] << (k < _s.location[j].retour.size() - 1 ? ", " : "");
		cout << endl;
		*/
}

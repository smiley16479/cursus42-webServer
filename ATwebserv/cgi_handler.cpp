#include "cgi_handler.hpp"

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

void	go_cgi(std::map<std::string, std::vector<std::string> >& mp, std::vector<server_info>& serv)
{
	cgi_handler	cgi(mp, serv);
//	cgi_handler	cgi(av);
	std::string	tmp;
	int			pos;
	int			bfd[2];
	int			fd[2];
	pid_t		pid;
//	char		**plop = new char*[cgi.args.size() + 1];//{ (char*)"files/cgi/php-cgi", NULL };
	char		*plop = (char*)CGI;
	char		**lol = new char*[cgi.env.size() + 1];//{ (char*)"files/cgi/php-cgi", NULL };
	int	i;
//	std::string		out;

/*
	i = 0;
	for (std::vector<std::string>::iterator it = cgi.args.begin(); it != cgi.args.end(); it++, i++)
		plop[i] = (char*)it->c_str();
	plop[i] = NULL;
	*/
	i = 0;
	for (std::vector<std::string>::iterator it = cgi.env.begin(); it != cgi.env.end(); it++, i++)
		lol[i] = (char*)it->c_str();
	lol[i] = NULL;
	bfd[0] = dup(STDIN_FILENO);
	bfd[1] = dup(STDOUT_FILENO);
	if (pipe(fd) == -1)
		return ;
//		return (CRASH_PIPE);
	pid = fork();
	if (pid == -1)
		return ;
//		return (CRASH_FORK);
	if (pid == 0)
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		execve(plop, &plop, lol);
		close(STDOUT_FILENO);
		exit(1);
	}
	else
	{
//		for (int i = 0; plop[i]; i++)
//			printf("plop=%s\n", plop[i]);
		close(fd[1]);
		if (dup2(fd[0], STDIN_FILENO) == -1)
			return ;
		close(fd[0]);
		waitpid(pid, NULL, 0);
//		out << (char*)HEADER;
		while (getline(std::cin, tmp))
		{
			tmp += "\n";
			if ((pos = tmp.find("Content-type: ")) != std::string::npos)
			{
				tmp = tmp.replace(0, strlen("Content-type: "), "");
				mp["Content-Type:"].clear();
				mp["Content-Type:"].push_back(tmp);
			}
			else
			{
				mp["BODY"].push_back(tmp);
			}
		}
		dup2(bfd[0], STDIN_FILENO);
		dup2(bfd[1], STDOUT_FILENO);
		close(bfd[0]);
		close(bfd[1]);
//		out << (char*)FOOTER;
	}
	mp["A"].erase(mp["A"].begin());
	mp["A"].erase(mp["A"].begin());
	mp["A"].push_back(" 200");
	mp["A"].push_back(" OK\n");
//	delete [] plop;
	delete [] lol;
}

cgi_handler::cgi_handler()	{
	handler = (char*)CGI;
	args.push_back((char*)handler.c_str());
	args.push_back((char*)SCRIPT);
}

cgi_handler::cgi_handler(std::map<std::string, std::vector<std::string> >& mp, std::vector<server_info>& serv)	{
	int	pos;
	std::string	query(mp["A"][1]);
	std::string	path;
	std::string	tmp;

	handler = (char*)CGI;
	args.push_back((char*)handler.c_str());
//	args.push_back((char*)SCRIPT);

	if ((pos = query.find("?")) != std::string::npos)
	{
		path = "files";
		path += query.substr(0, pos);
		args.push_back(path);
		query = query.substr(pos + 1, std::string::npos);
		while ((pos = query.find("&")) != std::string::npos)
		{
			tmp = query.substr(0, pos);
			query = query.substr(pos + 1, std::string::npos);
			args.push_back(tmp);
		}
		query = query.substr(pos + 1, std::string::npos);
		args.push_back(query);
	}
	else
	{
	/*
		for (std::vector<std::string>::iterator it = vec.begin();
			it  != vec.end(); it++)
		{
			args.push_back((*it));
		}
		*/
//		std::cout << "vec = " << vec[1] << std::endl;
		path = "files";
		path += mp["A"][1];
		args.push_back(path);
	}
	if (!mp["BODY"].empty())
	{
		for (std::vector<std::string>::iterator it = mp["BODY"].begin(); it != mp["BODY"].end(); it++)
			tmp += *it;
		args.push_back(tmp);
	}
	extract_env(mp, serv);
}

cgi_handler::cgi_handler(const cgi_handler& other)	{
	handler = (char*)CGI;
	script = other.script;
	args = other.args;
}

cgi_handler::~cgi_handler()	{
	handler.clear();
	script.clear();
	args.clear();
	env.clear();
}

cgi_handler&	cgi_handler::operator=(const cgi_handler& other)	{
	handler = (char*)CGI;
	script = other.script;
	args = other.args;
	return (*this);
}

void	cgi_handler::extract_env(std::map<std::string, std::vector<std::string> >& mp, std::vector<server_info>& _s)
{
	std::string			tmp;
	std::string			buf(mp["A"][1]);
	std::string			var;
	size_t				pos;

	for (size_t i = 0; i < _s.size(); i++)
	{
		tmp =  "REDIRECT_STATUS=CGI";
		env.push_back(tmp);
		tmp = "SERVER_SOFTWARE=";
		for (size_t j = 0; j < _s[i].server_name.size(); j++)
			tmp+= _s[i].server_name[j];
		env.push_back(tmp);
		tmp = "SERVER_NAME=";
		tmp += _s[i].host;
		env.push_back(tmp);
		tmp = "GATEWAY_INTERFACE=CGI/1.1";
		env.push_back(tmp);
		tmp = "SERVER_PROTOCOL=HTTP/1.1";
		env.push_back(tmp);
		tmp = "SERVER_PORT=";
		tmp += _s[i].port;
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
		tmp += _s[i].location[0].root;
		tmp += (buf[0] == '/' ? buf.substr(1) : buf);
		env.push_back(tmp);
		tmp = "SCRIPT_NAME=";
		tmp += _s[i].location[0].root;
		tmp += (buf[0] == '/' ? buf.substr(1) : buf);
//		tmp += buf.substr(buf.find_last_of("/") + 1);
		env.push_back(tmp);
		tmp = "QUERY_STRING=";
		if ((pos = var.find("?")) != std::string::npos)
			tmp += var.substr(pos + 1);
		if (!mp["BODY"].empty())
		{
			for (std::vector<std::string>::iterator it = mp["BODY"].begin(); it != mp["BODY"].end(); it++)
				tmp += *it;
			mp["BODY"].clear();
		}
		env.push_back(tmp);
		tmp = "REMOTE_HOST=";
		if (!mp["Host:"].empty())
		{
			for (size_t j = 0; j < mp["Host:"].size(); j++)
				tmp+= mp["Host:"][j];
		}
		env.push_back(tmp);
		/*
		tmp = "REMOTE_ADDR=";
		tmp += args[args.size() -1].substr(args[args.size() -1].find("=")).substr(1, args[args.size() -1].substr(args[args.size() -1].find("=")).find(":") - 1);
		env.push_back(tmp);
		*/
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
//		tmp = "HTTP_COOKIE=";
		//GET COOKIE FROM SERVER
		//C EST UN BONUS !!!
//		env.push_back(tmp);
		tmp = "HTTP_REFERER=";
		if (!mp["Referer:"].empty())
		{
			for (size_t j = 0; j < mp["Referer:"].size(); j++)
				tmp+= mp["Referer:"][j];
		}
		env.push_back(tmp);
		/*
			cout << "auth_basic : " << _s[i].location[j].auth_basic << endl;
			cout << "auth_user_file : " << _s[i].location[j].auth_user_file << endl;
			cout << "autoindex : " << _s[i].location[j].autoindex << endl;
			cout << "return_directive : " << _s[i].location[j].return_directive << endl;
			cout << "allowed_method : ";
			for (size_t k = 0; k < _s[i].location[j].allowed_method.size(); k++)
				cout << _s[i].location[j].allowed_method[k] << (k < _s[i].location[j].allowed_method.size() - 1 ? ", " : "");
			cout << endl << "return : ";
			for (size_t k = 0; k < _s[i].location[j].retour.size(); k++)
				cout << _s[i].location[j].retour[k] << (k < _s[i].location[j].retour.size() - 1 ? ", " : "");
			cout << endl;
			*/
	}
}

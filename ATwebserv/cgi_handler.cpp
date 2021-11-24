#include "cgi_handler.hpp"

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
	std::string	lol;
	int			pos;
	int			bfd[2];
	int			fd[2];
	pid_t		pid;
	char		**plop = new char*[cgi.args.size() + 1];//{ (char*)"files/cgi/php-cgi", NULL };
	int	i;
//	std::string		out;

	i = 0;
	for (std::vector<std::string>::iterator it = cgi.args.begin(); it != cgi.args.end(); it++, i++)
		plop[i] = (char*)it->c_str();
	plop[i] = NULL;
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
		execve(plop[0], plop, NULL);
		close(STDOUT_FILENO);
	}
	else
	{
		for (int i = 0; plop[i]; i++)
			printf("plop=%s\n", plop[i]);
		close(fd[1]);
		if (dup2(fd[0], STDIN_FILENO) == -1)
			return ;;
		close(fd[0]);
//		out << (char*)HEADER;
		while (getline(std::cin, lol))
		{
			lol += "\n";
			if ((pos = lol.find("Content-type: ")) != std::string::npos)
			{
				lol = lol.replace(0, strlen("Content-type: "), "");
				mp["Content-Type:"].clear();
				mp["Content-Type:"].push_back(lol);
			}
			else
			{
				mp["BODY"].push_back(lol);
			}
		}
		(waitpid(pid, NULL, 0));
		dup2(bfd[0], STDIN_FILENO);
		dup2(bfd[1], STDOUT_FILENO);
//		out << (char*)FOOTER;
	}
	mp["A"].erase(mp["A"].begin());
	mp["A"].erase(mp["A"].begin());
	mp["A"].push_back(" 200");
	mp["A"].push_back(" OK\n");
	delete [] plop;
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
	/*
	else if ((pos = query.find(" ")) != std::string::npos)
	{
		path = query.substr(0, pos);
		args.push_back(path);
		query = query.substr(pos + 1, std::string::npos);
		while ((pos = query.find(" ")) != std::string::npos)
		{
			tmp = query.substr(0, pos);
			query = query.substr(pos + 1, std::string::npos);
			args.push_back(tmp);
		}
		query = query.substr(pos + 1, std::string::npos);
		args.push_back(query);
	}
	*/
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
}

cgi_handler::cgi_handler(const cgi_handler& other)	{
	handler = (char*)CGI;
	script = other.script;
	args = other.args;
}

cgi_handler::~cgi_handler()	{
}

cgi_handler&	cgi_handler::operator=(const cgi_handler& other)	{
	handler = (char*)CGI;
	script = other.script;
	args = other.args;
	return (*this);
}

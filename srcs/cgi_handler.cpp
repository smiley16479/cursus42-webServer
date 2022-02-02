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

int	is_cgi(std::vector<std::string>& query, std::vector<std::string>& extensions)
{
	int	i = 0;
	for (std::vector<std::string>::iterator type = extensions.begin(); type != extensions.end(); type++, i++)
	{
		for (std::vector<std::string>::iterator it = query.begin(); it != query.end(); it++)
		{
			if ((it->find(*type)) != std::string::npos)
				return (i);
		}
	}
	return (-1);
}

pid_t	go_cgi(int (*rfd)[2], std::string cgi_path, std::vector<std::string>& env)
{
	std::string	tmp;
	int			fd[2];
	int			bfd[2];
	pid_t		pid;
	char		*e_path[4];
	char		**c_env = new char*[env.size() + 1];
	int	i;

#ifdef _debug_
	std::cout << "Launching cgi at : " << cgi_path << std::endl;
#endif
	tmp = cgi_path;
	e_path[0] = (char*)tmp.c_str();
	if (cgi_path.find("php-cgi") != std::string::npos)	{
		e_path[1] = (char*)"-c";
		e_path[2] = (char*)"files/cgi/php.ini";
		e_path[3] = NULL;
	}
	else
		e_path[1] = NULL;
	i = 0;
	for (std::vector<std::string>::iterator it = env.begin(); it != env.end(); it++, i++)
	{
		std::cout << GREEN << it->c_str() << RESET << std::endl;
		c_env[i] = (char*)it->c_str();
	}
	c_env[i] = NULL;
	if (pipe(fd) == -1)
		throw std::runtime_error("ERROR IN PIPE ATTRIBUTION");
	if (pipe(bfd) == -1)
		throw std::runtime_error("ERROR IN PIPE ATTRIBUTION");
	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	fcntl(bfd[1], F_SETFL, O_NONBLOCK);
	pid = fork();
	if (pid == -1)
		throw std::runtime_error("ERROR IN FORK PROCESS");
	if (pid == 0)
	{
		close(fd[0]);
		close(bfd[1]);
		dup2(bfd[0], STDIN_FILENO);
		close(bfd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		execve(tmp.c_str(), e_path, c_env);
		exit(1);
	}
	else
	{
		close(bfd[0]);
		close(fd[1]);
	}
	delete [] c_env;
	(*rfd)[0] = fd[0];
	(*rfd)[1] = bfd[1];
	return (pid);
}

size_t	getcLen(std::vector<std::string>& env)	{
	size_t		pos;
	size_t		_cLen = 0;

	for (std::vector<std::string>::iterator it = env.begin(); it != env.end(); ++it)
	{
		if ((pos = it->find("CONTENT_LENGTH=")) != std::string::npos)
		{
			std::stringstream	ss(it->substr(strlen("CONTENT_LENGTH=")));
			ss >> _cLen;
			return (_cLen);
		}
	}
	return (0);
}

pid_t	go_cgi_fd(int (*rfd)[2], std::string cgi_path, std::vector<std::string>& env, std::string& body)
{
	std::string	tmp;
	pid_t		pid;
	int			fd[2];
	char		*e_path[4];
	char		**c_env = new char*[env.size() + 1];
	int	i;

	std::cout << "Launching cgi at : " << cgi_path << std::endl;
	tmp = cgi_path;
	e_path[0] = (char*)tmp.c_str();
	if (cgi_path.find("php-cgi") != std::string::npos)	{
		e_path[1] = (char*)"-c";
		e_path[2] = (char*)"files/cgi/php.ini";
		e_path[3] = NULL;
	}
	else
		e_path[1] = NULL;
	i = 0;
	for (std::vector<std::string>::iterator it = env.begin(); it != env.end(); it++, i++)
	{
#ifdef _debug_
		std::cout << GREEN << it->c_str() << RESET << std::endl;
#endif
		c_env[i] = (char*)it->c_str();
	}
	c_env[i] = NULL;
	if (pipe(fd) == -1)
		return (-1);
	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	pid = fork();
	if (pid == -1)
		return (-1);
	if (pid == 0)
	{
		while (!body.empty())
		{
			i = 0;
			i = write((*rfd)[1], body.substr(0, MAX_LEN).c_str(), body.substr(0, MAX_LEN).length());
			if (i != -1)
				body = body.substr(i);
			else
			{
				std::cerr << RED "Cgi write error" RESET << std::endl;
				break ;
			}
		}
		lseek((*rfd)[1], 0, SEEK_SET);
		dup2((*rfd)[1], STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		execve(tmp.c_str(), e_path, c_env);
		exit(1);
	}
	else
		close(fd[1]);
	delete [] c_env;
	(*rfd)[0] = fd[0];
	return (pid);
}

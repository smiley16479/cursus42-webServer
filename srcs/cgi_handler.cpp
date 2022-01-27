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
	char		**c_env = new char*[env.size() + 1];//{ (char*)"files/cgi/php-cgi", NULL };
	int	i;
//	std::string		out;

	std::cout << "Launching cgi at : " << cgi_path << std::endl;
	tmp = cgi_path;
	e_path[0] = (char*)tmp.c_str();
//	e_path[1] = NULL;
	e_path[1] = (char*)"-c";
	e_path[2] = (char*)"files/scripts/php.ini";
	e_path[3] = NULL;
	i = 0;
	for (std::vector<std::string>::iterator it = env.begin(); it != env.end(); it++, i++)
	{
		std::cout << GREEN << it->c_str() << RESET << std::endl;
		c_env[i] = (char*)it->c_str();
	}
	c_env[i] = NULL;
	if (pipe(fd) == -1)
		return (-1);
	if (pipe(bfd) == -1)
		return (-1);
//		return (CRASH_PIPE);
//	int	opt = 1;
//	setsockopt(fd[0], SOL_SOCKET, SOCK_NONBLOCK, &opt, sizeof(int));
//	setsockopt(fd[1], SOL_SOCKET, SOCK_NONBLOCK, &opt, sizeof(int));
//	setsockopt(bfd[0], SOL_SOCKET, SOCK_NONBLOCK, &opt, sizeof(int));
//	setsockopt(bfd[1], SOL_SOCKET, SOCK_NONBLOCK, &opt, sizeof(int));
	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	fcntl(fd[1], F_SETFL, O_NONBLOCK);
	fcntl(bfd[0], F_SETFL, O_NONBLOCK);
	fcntl(bfd[1], F_SETFL, O_NONBLOCK);
	pid = fork();
	if (pid == -1)
		return (-1);
//		return (CRASH_FORK);
	if (pid == 0)
	{
		close(fd[0]);
		close(bfd[1]);
		dup2(bfd[0], STDIN_FILENO);
//		close(STDOUT_FILENO);
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
	std::cout << "Pipe status in go cgi:" << std::endl;
	std::cout << "rfd[0] : " << fcntl((*rfd)[0], F_GETFD) << std::endl;
	std::cout << "rfd[1] : " << fcntl((*rfd)[1], F_GETFD) << std::endl;


/*
	int	status;
	int	plop;

	plop = waitpid(pid, &status, WNOHANG);
	std::cout << "Cgi status in go_cgi is : " << plop << std::endl;
	std::cout << "WIFEXITED(status)" << (WIFEXITED(status) ? "true" : "false") << std::endl;
	std::cout << "WEXITSTATUS(status)" << WEXITSTATUS(status) << std::endl;
	std::cout << "WIFSIGNALED(status)" << WIFSIGNALED(status) << std::endl;
	std::cout << "WTERMSIG(status)" << WTERMSIG(status) << std::endl;
	std::cout << "WCOREDUMP(status)" << WCOREDUMP(status) << std::endl;
	std::cout << "WIFSTOPPED(status)" << WIFSTOPPED(status) << std::endl;
	std::cout << "WSTOPSIG(status)" << WSTOPSIG(status) << std::endl;
	std::cout << "WIFCONTINUED(status)" << WIFCONTINUED(status) << std::endl;
	*/


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

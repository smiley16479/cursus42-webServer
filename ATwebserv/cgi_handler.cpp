#include "cgi_handler.hpp"
#include "color.hpp"
using namespace std;

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
	cout << BLUE "IN IS_CGI() return :" RESET;
	int	i = 0;
	for (std::vector<std::string>::iterator type = extensions.begin(); type != extensions.end(); type++, i++)
	{
		for (std::vector<std::string>::iterator it = query.begin(); it != query.end(); it++)
		{
			if ((it->find(*type)) != std::string::npos){
				cout << i << endl;
				return (i);
			}
		}
	}
	cout << -1 << endl;
	return (-1);
}

int	go_cgi(int (*rfd)[2], std::string cgi_path, std::vector<std::string>& env)
{
	cout << BLUE "IN GO_CGI()\n" RESET;
	int			fd[2];
	int			bfd[2];
	pid_t		pid;
	char		*e_path[4];
	char		**c_env = new char*[env.size() + 1];//{ (char*)"files/cgi/php-cgi", NULL };
	int	i;
//	std::string		out;

	e_path[0] = (char*)cgi_path.c_str();
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
	if (pipe(fd) == -1 || pipe(bfd) == -1)
		return (-1);
//		return (CRASH_PIPE);
//	int	opt = 1;
//	setsockopt(fd[0], SOL_SOCKET, SOCK_NONBLOCK, &opt, sizeof(int));
//	setsockopt(fd[1], SOL_SOCKET, SOCK_NONBLOCK, &opt, sizeof(int));
	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	fcntl(fd[1], F_SETFL, O_NONBLOCK);
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
		cout << GREEN "CGI_FORK OK !\n" RESET;
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		execve(cgi_path.c_str(),e_path, c_env);
		exit(1);
	}
	else
	{
		close(bfd[0]); // read
		close(fd[1]); // write
	}
	delete [] c_env;
	(*rfd)[0] = fd[0];
	(*rfd)[1] = bfd[1];
	return (0);
}

size_t	getcLen(std::vector<std::string>& env)	{
	cout << BLUE "IN GETCLEN()\n" RESET;
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

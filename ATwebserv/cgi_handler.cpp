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

int	go_cgi(std::string cgi_path, std::vector<std::string>& post_args, std::vector<std::string>& env)
{
	size_t		_cLen;
	std::string	tmp;
	int			fd[2];
	int			bfd[2];
	pid_t		pid;
	char		*e_path[2] = { NULL, NULL };
	char		**c_env = new char*[env.size() + 1];//{ (char*)"files/cgi/php-cgi", NULL };
	int	i;
//	std::string		out;

	_cLen = getcLen(env);
//	e_path[0] = cgi_path.c_str();
//	e_path[1] = (char*)CGI_MODE;
//	e_path[2] = NULL;
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
	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	fcntl(fd[1], F_SETFL, O_NONBLOCK);
	pid = fork();
	if (pid == -1)
		return (-1);
//		return (CRASH_FORK);
	if (pid == 0)
	{
		close(fd[0]);
		if (!post_args.empty())
		{
			for (std::vector<std::string>::iterator it = post_args.begin(); it != post_args.end(); ++it)
			{
				//	std::cerr << "post_args = " << *it << std::endl;
				size_t post = it->size();
				std::cout << "post: " << post << std::endl;
				std::cout << "cLen: " << _cLen << std::endl;
				if (post > _cLen)
				{
					post = _cLen;
					_cLen = 0;
				}
				else
					_cLen -= post;
				if (!it->empty())
				{
					write(bfd[1], it->c_str(), post);
				}
				if (_cLen == 0)
					break ;
			}
		}
		close(bfd[1]);
		dup2(bfd[0], STDIN_FILENO);
		close(bfd[0]);
//		close(STDOUT_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		execve(cgi_path.c_str(),e_path, c_env);
		exit(1);
	}
	else
	{
		close(bfd[0]);
		close(bfd[1]);
		close(fd[1]);
	}
	delete [] c_env;
	return (fd[0]);
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

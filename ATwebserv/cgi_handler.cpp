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

bool	is_cgi(std::vector<std::string>& query, std::vector<std::string>& extensions)
{
	for (std::vector<std::string>::iterator type = extensions.begin(); type != extensions.end(); type++)
	{
		for (std::vector<std::string>::iterator it = query.begin(); it != query.end(); it++)
		{
			if ((it->find(*type)) != std::string::npos)
				return (true);
		}
	}
	return (false);
}


static int		line_init(char **line, int offset, int ret)
{
	if (!(*line = (char*)malloc((offset + 1) * sizeof(char))))
		return (-1);
	((*line)[offset]) = '\0';
	return (ret);
}

static int		recursive_get_line(char **line, int fd, int offset)
{
	char	buffer;
	int		ret;

	ret = read(fd, &buffer, 1);
	if (ret == 1)
	{
		if (buffer == '\n' || buffer == '\0')
			return (line_init(line, offset, 1));
		else
		{
			ret = recursive_get_line(line, fd, offset + 1);
			if ((*line) != NULL)
				((*line)[offset]) = buffer;
			return (ret);
		}
	}
	else if (ret == 0)
		return (line_init(line, offset, 0));
	return (-1);
}

int				rec_gnl(int fd, char **line)
{
	char	buffer;

	if (fd < 0 || !line || read(fd, &buffer, 0) < 0)
		return (-1);
	*line = NULL;
	return (recursive_get_line(line, fd, 0));
}


void	go_cgi(std::vector<std::string>& post_args, std::string& body, std::vector<std::string>& env, int fd_in)
{
	size_t		_cLen;
	std::string	tmp;
	size_t		pos;
	int			fd[2];
	int			bfd[2];
	pid_t		pid;
	char		**e_path = new char*[2];
	char		**c_env = new char*[env.size() + 1];//{ (char*)"files/cgi/php-cgi", NULL };
	int	i;
//	std::string		out;

	_cLen = getcLen(env);
	e_path[0] = (char*)CGI;
	e_path[1] = NULL;
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
		return ;
	if (pipe(bfd) == -1)
		return ;
//		return (CRASH_PIPE);
	pid = fork();
	if (pid == -1)
		return ;
//		return (CRASH_FORK);
	if (pid == 0)
	{
		close(fd[0]);
		if (!post_args.empty())
		{
			for (std::vector<std::string>::iterator it = post_args.begin(); it != post_args.end(); it++)
			{
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
//					write(2, it->c_str(), post);
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
		execve(e_path[0],e_path, c_env);
		exit(1);
	}
	else
	{
		close(bfd[0]);
		close(bfd[1]);
		close(fd[1]);
		waitpid(pid, NULL, 0);
		if (!body.empty())
			body.clear();
		post_args.clear();
		dup2(fd[0], fd_in);
		close(fd[0]);
		char *sd;
		while (rec_gnl(fd_in, &sd) != 0)
		{
			tmp = (char*)sd;
			std::cout << tmp << std::endl;
			if ((pos = tmp.find("Content-type: ")) != std::string::npos)
			{
				tmp.replace(0, strlen("Content-type: "), "");
				post_args.push_back(tmp);
			}
			else if (!((pos = tmp.find("X-Powered-By:")) != std::string::npos))
			{
				if (body.empty()) 
					body = tmp;
				else
					body.append(tmp);
			}
			free(sd);
		}
	}
	delete [] e_path;
	delete [] c_env;
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

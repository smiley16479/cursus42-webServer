#include "cgi_handler.hpp"

int	main(int ac, char **av, char **env)
{
//	cgi_handler	cgi(&av[1]);
	cgi_handler	cgi(av);
	std::string	lol;
	int			bfd[2];
	int			fd[2];
	pid_t		pid;
	char		**plop = cgi.args.data();//{ (char*)"files/cgi/php-cgi", NULL };
	std::ofstream		out("out.html");

	bfd[0] = dup(STDIN_FILENO);
	bfd[1] = dup(STDOUT_FILENO);
	if (pipe(fd) == -1)
		return (CRASH_PIPE);
	pid = fork();
	if (pid == -1)
		return (CRASH_FORK);
	if (pid == 0)
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		execve(cgi.handler.c_str(), plop, NULL);
		//close(STDOUT_FILENO);
	}
	else
	{
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
//		out << (char*)HEADER;
		while (getline(std::cin, lol))
			out << lol << std::endl;
		(waitpid(pid, NULL, 0));
		dup2(bfd[0], STDIN_FILENO);
		dup2(bfd[1], STDOUT_FILENO);
//		out << (char*)FOOTER;
	}
	return (0);
}

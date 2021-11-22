#include "cgi_handler.hpp"

int	main(int ac, char **av, char **env)
{
	cgi_handler	cgi;
	pid_t		pid;
	char		**plop = cgi.args.data();//{ (char*)"files/cgi/php-cgi", NULL };

	pid = fork();
	if (pid == 0)
		execve(cgi.handler.c_str(), plop, NULL);
	else
		(waitpid(pid, NULL, 0));
	return (0);
}

#include "cgi_handler.hpp"

void	go_cgi(std::map<std::string, std::vector<std::string> mp)
{

}

cgi_handler::cgi_handler()	{
	handler = (char*)CGI;
	args.push_back((char*)handler.c_str());
	args.push_back((char*)SCRIPT);
}

cgi_handler::cgi_handler(char **av)	{
	int	pos;
	std::string	query((char*)av[0]);
	std::string	path;
	std::string	tmp;

	handler = (char*)CGI;
	args.push_back((char*)handler.c_str());
//	args.push_back((char*)SCRIPT);

	if ((pos = query.find("?")) != std::string::npos)
	{
		path = query.substr(0, pos);
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
		for (int i = 0; av[i] != NULL; i++)
			args.push_back((char*)av[i]);
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

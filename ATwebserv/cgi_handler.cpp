#include "cgi_handler.hpp"

cgi_handler::cgi_handler()	{
	handler = (char*)CGI;
	args.push_back((char*)handler.c_str());
	args.push_back((char*)SCRIPT);
}

cgi_handler::cgi_handler(char **av)	{
	int	pos;
	std::string	query(av[1]);
	std::string	tmp(query);

	handler = (char*)CGI;
	args.push_back((char*)handler.c_str());
//	args.push_back((char*)SCRIPT);

/*
	for (int i = 0; av[i] != NULL; i++)
	{
		query += (char*)av[i];
		query += "&";
	}
	args.push_back((char*)query.c_str());
	*/
	if ((pos = query.find("?", 1)) > 0)
	{
		tmp = query.substr(0, pos);
		args.push_back((char*)query.c_str());
		tmp = tmp.substr(pos, std::string::npos);
		while ((pos = tmp.find("&", 1)) > 0)
		{
			tmp.substr(pos, std::string::npos);
			args.push_back((char*)tmp.c_str());
		}
	}
	else
	{
		for (int i = 0; av[i] != NULL; i++)
			args.push_back(av[i]);
	}
	args.push_back(NULL);
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

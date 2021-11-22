#include "cgi_handler.hpp"

cgi_handler::cgi_handler()	{
	handler = (char*)CGI;
	args.push_back((char*)handler.c_str());
	args.push_back((char*)SCRIPT);
}

cgi_handler::cgi_handler(char **av)	{
	std::string	query;

	handler = (char*)CGI;
	args.push_back((char*)handler.c_str());
	args.push_back((char*)SCRIPT);

/*
	for (int i = 0; av[i] != NULL; i++)
	{
		query += (char*)av[i];
		query += "&";
	}
	args.push_back((char*)query.c_str());
	*/
	for (int i = 0; av[i] != NULL; i++)
		args.push_back(av[i]);
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

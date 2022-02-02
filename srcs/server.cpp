#include "server.hpp"
#include "config_checker.hpp"
#include "request_handler.hpp"
#include "client_handler.hpp"
#include <stdexcept>

#include <unistd.h> // Pour close, À ENLEVER

using namespace std;

server::server(std::string av)
{
	config_checker confCheck;
	confCheck.check_conFile(av);
	this->_s = confCheck._si; // je voulais le passer par reference mais pas possible av l'architecture actuelle
}

server::~server()
{
#ifdef _debug_
	std::cout << RED "server destructeur..." RESET << endl;
#endif
}

void server::initialize(void) {
	struct sockaddr_in servaddr;

	if ((_epoll._epoll_fd = epoll_create(1)) == -1) /* INITIALISATION DE L'INSTANCE EPOLL */
		throw std::runtime_error("ERROR IN EPOLL INSTANCE CREATION");
/* INITIALISATION DES SOCKET DES SERVER(S) VIRTUEL(S) */
	for (size_t i = 0; i < _s.size(); i++) {
		bool skip = false;
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET; 
		servaddr.sin_addr.s_addr = htonl(0); //127.0.0.1 -> 2130706433
		servaddr.sin_port = htons(atoi(_s[i].port.c_str()));
/* VERIFIE QUE LE SERVER A BIND N'AIT PAS D'HOMOLOGUE */
		for (int j = i - 1; i && j >= 0; --j)
				if (_s[j].host == _s[i].host && _s[j].port == _s[i].port && (skip = true))
					break ;
		if (skip)
			continue ;
/* CREATION DU SERVER */
		int	opt = 1;
		if ((_s[i].socket = socket(AF_INET, SOCK_STREAM | O_NONBLOCK, 0)) < 0 
				|| setsockopt(_s[i].socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int))
				|| bind(_s[i].socket, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 
				|| listen(_s[i].socket, 0) < 0 )
			throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");
/* && AJOUT DE CES DERNIERS À L'INSTANCE EPOLL */
		opt = EPOLL_CTL_ADD;
		struct epoll_event ev;
		bzero(&ev, sizeof(ev));
		ev.events = EPOLLIN;
		ev.data.fd = _s[i].socket;
#ifdef _debug_
		std::cout << "Adding socket fd_" << _s[i].socket << " to epoll interest list" << std::endl;
#endif
		if(epoll_ctl(_epoll._epoll_fd, opt, _s[i].socket, &ev))
		{
			throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
		}
	}
}

void server::run(void) {
	initialize();
	std::vector<int>	chunks;
	request_handler		header(_s);
	client_handler		client;
	client_info*		ptr;
	int					serv_id;

	while(1)
	{
		_epoll._event_count = epoll_wait(_epoll._epoll_fd, _epoll._events, MAX_EVENTS, 0);
		for(int i = 0; i < _epoll._event_count; ++i) {
			if ((serv_id = is_new_client(_epoll._events[i].data.fd)) >= 0 && _epoll._events[i].events & EPOLLIN) {
				client.add(_epoll, get_time_out(serv_id), i);
				std::cout << "New client added !\n" << std::endl;
			}
			else
			{
				ptr = client.get_info(_epoll._events[i].data.fd);
				if (_epoll._events[i].events & EPOLLIN) {
					if (ptr != NULL)
						ptr->fd_in(header);
				}
				else if (_epoll._events[i].events & EPOLLOUT)	{
					if (ptr != NULL)
						ptr->fd_out(header);
				}
			}
		}
		client.check_all_timeout();
	}

	if(close(_epoll._epoll_fd))
	{
		std::cerr << "Failed to close epoll file descriptor" << std::endl;
		throw std::runtime_error("ERROR IN FD (CLOSE) MANIPULATION");
	}
	return;
}

int server::is_new_client(int fd) {
	for (size_t i = 0; i < _s.size(); ++i)
	{
		if (fd == _s[i].socket)
			return i;
	}
	return -1;
}

int server::get_time_out(int id_serv) {
	return atoi(_s[id_serv].time_out.c_str());
};

/*
* AFFICHE TOUTES LES INFORMATIONS CONTENUES DS LES STRUCTURES GÉNÉRÉES PAR LE FICHIER DE .CONF
*/
void server::display_server(void)
{
	for (size_t i = 0; i < _s.size(); i++)
	{
		std::cout << GREEN ITALIC UNDERLINE "DISPLAY SERVER INFORMATION" RESET GREEN " :" RESET << endl;
		std::cout << "server_name : " << _s[i].server_name << endl;
		std::cout << "time_out : " << _s[i].time_out << endl;
		std::cout << "port : " << _s[i].port << endl;
		std::cout << "host : " << _s[i].host << endl;
		std::cout << "error_page : " << _s[i].error_page << endl;
		std::cout << "max_file_size : " << _s[i].max_file_size << endl;
		for (size_t j = 0; j < _s[i].location.size(); j++) {
			std::cout << GREEN "LOCATION : " RESET << endl;
			std::cout << "location : " << _s[i].location[j].location << endl;
			std::cout << "auth_basic : " << _s[i].location[j].auth_basic << endl;
			std::cout << "auth_user_file : " << _s[i].location[j].auth_user_file << endl;
			std::cout << "autoindex : " << _s[i].location[j].autoindex << endl;
			std::cout << "index : " << _s[i].location[j].index << endl;
			std::cout << "max_file_size : " << _s[i].location[j].max_file_size << endl;
			std::cout << "return_directive : " << _s[i].location[j].return_directive << endl;
			std::cout << "root : " << _s[i].location[j].root << endl;
			std::cout << "upload_path : " << _s[i].location[j].upload_path << endl;
			std::cout << "cgi_path : " << _s[i].location[j].cgi_path << endl;
			for (size_t k = 0; k < _s[i].location[j].cgi_file_types.size(); k++)
				std::cout << "cgi_file_types : " << _s[i].location[j].cgi_file_types[k] << endl;
			std::cout << "allowed_method : ";
			for (size_t k = 0; k < _s[i].location[j].allowed_method.size(); k++)
				std::cout << _s[i].location[j].allowed_method[k] << (k < _s[i].location[j].allowed_method.size() - 1 ? ", " : "");
			std::cout << endl << "return : ";
			for (size_t k = 0; k < _s[i].location[j].retour.size(); k++)
				std::cout << _s[i].location[j].retour[k] << (k < _s[i].location[j].retour.size() - 1 ? ", " : "");
		std::cout << endl;
		}
	}
	std::cout << std::endl;
}

struct epoll_event*	get_event(struct_epoll& _epoll, int fd)
{
	for(int i = 0; i < _epoll._event_count; ++i) {
		if (_epoll._events[i].data.fd == fd)
			return (&_epoll._events[i]);
	}
	return (NULL);
}

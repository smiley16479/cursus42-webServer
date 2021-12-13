#include "server.hpp"
#include "config_checker.hpp"
#include "header_handler.hpp"
#include "client_handler.hpp"
#include <stdexcept>

#include <unistd.h> // Pour close, À ENLEVER

using namespace std;

server::server(std::string av)
{
	config_checker confCheck;
	confCheck.check_conFile(av);
	this->_s = *confCheck._si; // je voulais le passer par reference mais pas possible av l'architecture actuelle
}

server::~server()
{
	cout << RED "server destructeur..." RESET << endl;
}

void server::initialize(void) {
	struct sockaddr_in servaddr;

	if ((_epoll._epoll_fd = epoll_create(1)) == -1) /* INITIALISATION DE L'INSTANCE EPOLL */
		throw std::runtime_error("ERROR IN EPOLL INSTANCE CREATION");
/* INITIALISATION DES SOCKET DES SERVER(S) VIRTUEL(S) */
	for (size_t i = 0; i < _s.size(); i++) {
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET; 
		servaddr.sin_addr.s_addr = htonl(0); //127.0.0.1 -> 2130706433
		servaddr.sin_port = htons(atoi(_s[i].port.c_str()));
	//	if ((_s[i].socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 
		if ((_s[i].socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0 
				|| bind(_s[i].socket, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 
				|| listen(_s[i].socket, 0) < 0 )
			throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");
/* && AJOUT DE CES DERNIERS À L'INSTANCE EPOLL */
		//SET NON BLOCK
		int opt = 1;
		setsockopt(_s[i].socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
		if (fcntl(_s[i].socket, F_SETFL, O_NONBLOCK) == -1)
		{
			perror("fcntl F_SETFL, FNDELAY | FASYNC ");
			exit(EXIT_FAILURE);
		}
		//END
		_epoll._event.events = EPOLLIN;
		_epoll._event.data.fd = _s[i].socket;
		if(epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_ADD, _s[i].socket, &_epoll._event))
			throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}
}

void server::run(void) {
	initialize();
	std::vector<int>	chunks;
	header_handler header(_s);
	client_handler client;
	int serv_id;

	while(1)
	{
		//CHUNK RESOLUTION
		chunks = client.handle_chunks(_epoll);
		for (std::vector<int>::iterator it = chunks.begin(); it != chunks.end(); it++)
			response_handler(client, header, *it);
		chunks.clear();
		//END
		printf("\nPolling for input...\n");
		_epoll._event_count = epoll_wait(_epoll._epoll_fd, _epoll._events, MAX_EVENTS, 50); //500
		printf("%d ready events\n", _epoll._event_count);
		for(int i = 0; i < _epoll._event_count; ++i) {
			if ((serv_id = is_new_client(_epoll._events[i].data.fd)) >= 0 && (_epoll._events[i].events & EPOLLIN) == EPOLLIN) {
				client.add(_epoll, get_time_out(serv_id), i);
				printf("New client added\n");
			}
			else {
				bzero(str, sizeof(str)); // ON EFFACE UN HYPOTHÉTIQUE PRÉCÉDENT MSG
				// printf("client(fd : %d) msg : " YELLOW "\n%s\n" RESET,_events[i].data.fd,  str); 
				if (recv(_epoll._events[i].data.fd, str, sizeof(str), MSG_DONTWAIT) != -1)
				{
					client.rqst_append(_epoll._events[i].data.fd, str);
					response_handler(client, header, _epoll._events[i].data.fd);

		//				client.remove(_epoll, i);
				//		client.clear(_epoll._events[i].data.fd); // EFFACE LA PRÉCÉDENTE RQST, REMISE À ZERO DU TIME_OUT
					//	close(_epoll._events[i].data.fd); // DE FAÇON A FERMER LA CONNEXION MS JE SAIS PAS SI ÇA DOIT ETRE FAIT COMMME ÇA
				}
				else
					client.remove(_epoll, _epoll._events[i].data.fd);
				//	client.rearm(_epoll, client.get_info(_epoll._events[i].data.fd).time_out, _epoll._events[i].data.fd);
			}
		}
		client.check_all_timeout(_epoll);
		printf(RED "_event_count : %d\n" RESET, _epoll._event_count);
	}

	if(close(_epoll._epoll_fd))
	{
		fprintf(stderr, "Failed to close epoll file descriptor\n");
		throw std::runtime_error("ERROR IN FD (CLOSE) MANIPULATION");
	}
	return;
}

int server::is_new_client(int fd) {
	std::cout << "COUCOU FD = " << fd << std::endl;
	for (int i = 0; i < _s.size(); ++i)
	{
		if (fd == _s[i].socket)
			return i;
	}
	std::cout << "COUCOU FD = " << fd << std::endl;
	return -1;
}

int server::get_time_out(int id_serv) {
	return atoi(_s[id_serv].time_out.c_str());
};

void	display_loc(std::pair<const std::string, locati_info>& loc, int depth)
{
	cout << "location : " << loc.first << std::endl;
	if (!loc.second.location.empty())
	{
		for (int i = 0; i < depth; i++)
			std::cout << "\t";
		for (std::map<std::string, locati_info>::iterator it = loc.second.location.begin(); it != loc.second.location.end(); it++)
			display_loc(*it, depth + 1);
		cout << endl;
	}
}

/* 
* AFFICHE TOUTES LES INFORMATIONS CONTENUES DS LES STRUCTURES GÉNÉRÉES PAR LE FICHIER DE .CONF
*/
void server::display_server(void)
{
	for (size_t i = 0; i < _s.size(); i++)
	{
		cout << GREEN ITALIC UNDERLINE "DISPLAY SERVER INFORMATION" RESET GREEN " :" RESET << endl;
		for (size_t j = 0; j < _s[i].server_name.size(); j++)
			cout << "server_name : " << _s[i].server_name[j] << endl;
		cout << "time_out : " << _s[i].time_out << endl;
		cout << "port : " << _s[i].port << endl;
		cout << "host : " << _s[i].host << endl;
		cout << "error_page : " << _s[i].error_page << endl;
		cout << "max_file_size : " << _s[i].max_file_size << endl;
		for (size_t j = 0; j < _s[i].cgi_file_types.size(); j++)
			cout << "cgi_file_types : " << _s[i].cgi_file_types[j] << endl;
		for (std::map<std::string, locati_info>::iterator it = _s[i].location.begin(); it != _s[i].location.end(); it++) {
			cout << GREEN "LOCATION : " RESET << endl;
			display_loc(*it, 0);
			cout << "auth_basic : " << it->second.auth_basic << endl;
			cout << "auth_user_file : " << it->second.auth_user_file << endl;
			cout << "autoindex : " << it->second.autoindex << endl;
			cout << "index : " << it->second.index << endl;
			cout << "max_file_size : " << it->second.max_file_size << endl;
			cout << "return_directive : " << it->second.return_directive << endl;
			cout << "root : " << it->second.root << endl;
			cout << "allowed_method : ";
			for (size_t k = 0; k < it->second.allowed_method.size(); k++)
				cout << it->second.allowed_method[k] << (k < it->second.allowed_method.size() - 1 ? ", " : "");
			cout << endl << "return : ";
			for (size_t k = 0; k < it->second.retour.size(); k++)
				cout << it->second.retour[k] << (k < it->second.retour.size() - 1 ? ", " : "");
			cout << endl;
		}
	}
}

void	server::response_handler(client_handler& client, header_handler& header, int fd)	{
	if (client.is_request_fulfilled(fd)) {
		cout << "request_fulfilled !!\n";
		header.reader(/* str */client.get_rqst(fd).c_str()); // PROBLEME NE TRANSMET PLUS LES FAVICON D'INDEX_HTML
		header.writer();
		client.clear(fd);
		if (header.get_response().length() > MAX_LEN)
		{
			client.fill_resp(fd, header.get_response());
			client.chunked_resp(_epoll, fd);
		}
		else
		{
			if (send(fd, header.get_response().c_str(), header.get_response().length(), MSG_DONTWAIT) == -1)
			{
				client.remove(_epoll, fd);
//				client.rearm(_epoll, client.get_info(fd).time_out, fd);
			}
			else
				client.time_reset(_epoll, client.get_info(fd).time_out, fd);
		}
	}
	else
	{
		client.time_reset(_epoll, client.get_info(fd).time_out, fd);
	}
}

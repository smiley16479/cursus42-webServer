#include "server.hpp"
#include "config_checker.hpp"
#include "header_handler.hpp"
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

	if ((_epoll_fd = epoll_create(1)) == -1) /* INITIALISATION DE L'INSTANCE EPOLL */
		throw std::runtime_error("ERROR IN EPOLL INSTANCE CREATION");
/* INITIALISATION DES SOCKET DES SERVER(S) VIRTUEL(S) */
	for (size_t i = 0; i < _s.size(); i++) {
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET; 
		servaddr.sin_addr.s_addr = htonl(0); //127.0.0.1 -> 2130706433
		servaddr.sin_port = htons(atoi(_s[i].port.c_str()));
		if ((_s[i].socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 
				|| bind(_s[i].socket, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 
				|| listen(_s[i].socket, 0) < 0 )
			throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");
/* && AJOUT DE CES DERNIERS À L'INSTANCE EPOLL */
		_event.events = EPOLLIN;
		_event.data.fd = _s[i].socket;
		if(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _s[i].socket, &_event))
			throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}
}

void server::run(void) {
	initialize();
	header_handler header(_s);
	int i;

	while(1)
	{
		printf("\nPolling for input...\n");
		_event_count = epoll_wait(_epoll_fd, _events, MAX_EVENTS, 30000);
		printf("%d ready events\n", _event_count);
		for(int i = 0; i < _event_count; i++) {
			if (is_new_client(_events[i].data.fd) && (_events[i].events & EPOLLIN) == EPOLLIN) {
				int client_fd;
				struct sockaddr_in clientaddr;
				socklen_t len = sizeof(clientaddr);
				if ((client_fd = accept(_events[i].data.fd, (struct sockaddr *)&clientaddr, &len)) < 0)
					throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");
				clientaddr.sin_addr;
				_event.events = EPOLLIN;
				_event.data.fd = client_fd;
				if(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &_event)) {
					fprintf(stderr, "Failed to add file descriptor to epoll\n");
					// close(_epoll_fd);
					throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
				}
				printf("New client added\n");
			}
/*		else if ((events[i].events & EPOLLIN) == EPOLLIN) {
					printf("Reading file descriptor '%d' -- ", events[i].data.fd);
					bytes_read = read(events[i].data.fd, read_buffer, READ_SIZE);
					printf("%zd bytes read.\n", bytes_read);
					read_buffer[bytes_read] = '\0';
					printf("Read '%s'\n", read_buffer);
	
					if(!strncmp(read_buffer, "stop\n", 5))
						running = 0;
			}
			else {
						printf(YELLOW "Writing file descriptor '%d' -- \n" RESET, events[i].data.fd);
					//   for(i = 0; i < _event_count; i++)
				bytes_read = write(events[i].data.fd, read_buffer, READ_SIZE);
						printf("%zd bytes written.\n", bytes_read);
						read_buffer[bytes_read] = '\0';
						printf("Written '%s'\n", read_buffer);
			} */
			else {
				bzero(str, sizeof(str)); // ON EFFACE UN HYPOTHÉTIQUE PRÉCÉDENT MSG
				if (recv(_events[i].data.fd, str, sizeof(str), 0) <= 0) {
						printf("server: client just left\n");
						epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _events[i].data.fd, &_event);
						close(_events[i].data.fd);
						// break; // Pk Break T-ON ?
				}
				else { /* RECEPTION... ET TRAITEMENT DE LA REQUETE */
					// printf("client(fd : %d) msg : " YELLOW "\n%s\n" RESET,_events[i].data.fd,  str); 
					header.reader(str);
					header.writer();
					send(_events[i].data.fd, header.get_response().c_str(), header.get_response().length(), 0);
					// close(_events[i].data.fd); // DE FAÇON A FERMER LA CONNEXION MS JE SAIS PAS SI ÇA DOIT ETRE FAIT COMMME ÇA
				}
			}
		}
	printf(RED "_event_count : %d\n" RESET, _event_count);
	}

	if(close(_epoll_fd))
	{
		fprintf(stderr, "Failed to close epoll file descriptor\n");
		throw std::runtime_error("ERROR IN FD (CLOSE) MANIPULATION");
	}
	return;
}

bool server::is_new_client(int fd) {
	for (int i = 0; i < _s.size(); ++i)
		if (fd == _s[i].socket)
			return true;
	return false;
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
		for (size_t j = 0; j < _s[i].location.size(); j++) {
			cout << GREEN "LOCATION : " RESET << endl;
			cout << "location : " << _s[i].location[j].location << endl;
			cout << "auth_basic : " << _s[i].location[j].auth_basic << endl;
			cout << "auth_user_file : " << _s[i].location[j].auth_user_file << endl;
			cout << "autoindex : " << _s[i].location[j].autoindex << endl;
			cout << "index : " << _s[i].location[j].index << endl;
			cout << "max_file_size : " << _s[i].location[j].max_file_size << endl;
			cout << "return_directive : " << _s[i].location[j].return_directive << endl;
			cout << "root : " << _s[i].location[j].root << endl;
			cout << "allowed_method : ";
			for (size_t k = 0; k < _s[i].location[j].allowed_method.size(); k++)
				cout << _s[i].location[j].allowed_method[k] << (k < _s[i].location[j].allowed_method.size() - 1 ? ", " : "");
			cout << endl << "return : ";
			for (size_t k = 0; k < _s[i].location[j].retour.size(); k++)
				cout << _s[i].location[j].retour[k] << (k < _s[i].location[j].retour.size() - 1 ? ", " : "");
			cout << endl;
		}
	}
}








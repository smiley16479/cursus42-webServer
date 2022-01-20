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
		if ((_s[i].socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 
				|| bind(_s[i].socket, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 
				|| listen(_s[i].socket, 0) < 0 )
			throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");
/* && AJOUT DE CES DERNIERS À L'INSTANCE EPOLL */
		_epoll._event.events = EPOLLIN;
		_epoll._event.data.fd = _s[i].socket;
		if(epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_ADD, _s[i].socket, &_epoll._event))
			throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}
}

void server::run(void) {
	initialize();
	request_handler rqst(_s);
	client_handler client(_epoll);
	int byte_recved;
	int serv_id;

	while(1)
	{
		printf("\nPolling for input...\n");
		_epoll._event_count = epoll_wait(_epoll._epoll_fd, _epoll._events, MAX_EVENTS, 30000); //500
		printf("%d ready events\n", _epoll._event_count);
		for(int i = 0; i < _epoll._event_count; ++i) {
			if ((serv_id = is_new_client(i)) >= 0 && (_epoll._events[i].events & EPOLLIN) == EPOLLIN) {
				client.add(get_time_out(serv_id), i);
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
			} 
*/
			else if ( (_epoll._events[i].events & EPOLLIN) == EPOLLIN ) {
				bzero(str, sizeof(str)); // ON EFFACE UN HYPOTHÉTIQUE PRÉCÉDENT MSG
				if ((byte_recved = recv(_epoll._events[i].data.fd, str, sizeof(str), 0)) <= 0) {
						client.remove(i);
						printf("server: client just left\n");
						// break; // Pk Break T-ON ?
				}
				else { /* RECEPTION... ET TRAITEMENT DE LA REQUETE */
					// printf("client(fd : %d) msg : " YELLOW "\n%s\n" RESET,_events[i].data.fd,  str); 
					client.rqst_append(i, str, byte_recved);
					if (client.is_request_fulfilled(i)) {
						cout << "request_fulfilled !!\n";
						rqst.reader(client.get_info(i));
						rqst.writer();
						// send(_epoll._events[i].data.fd, rqst.get_response().c_str(), rqst.get_response().length(), 0);
						client.send(i); // send() FERME LA CONNEXION ET VIRER LE CLIENT MS JE SAIS PAS SI ÇA DOIT ETRE FAIT COMMME ÇA
					}
				}
			}
			else if ( (_epoll._events[i].events & EPOLLOUT) == EPOLLOUT ) {

			}
		}
		client.check_all_timeout();
		printf(RED "_event_count : %d\n" RESET, _epoll._event_count);
	}

	if(close(_epoll._epoll_fd))
	{
		fprintf(stderr, "Failed to close epoll file descriptor\n");
		throw std::runtime_error("ERROR IN FD (CLOSE) MANIPULATION");
	}
	return;
}

size_t server::is_new_client(int id) {
	for (size_t i = 0; i < _s.size(); ++i)
		if (_epoll._events[id].data.fd == _s[i].socket)
			return i;
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
		cout << GREEN ITALIC UNDERLINE "DISPLAY SERVER INFORMATION" RESET GREEN " :" RESET << endl;
		// for (size_t j = 0; j < _s[i].server_name.size(); j++) // Qd server_name etait un vector
		cout << "server_name : " << _s[i].server_name << endl;
		cout << "time_out : " << _s[i].time_out << endl;
		cout << "port : " << _s[i].port << endl;
		cout << "host : " << _s[i].host << endl;
		cout << "error_page : " << _s[i].error_page << endl;
		cout << "max_file_size : " << _s[i].max_file_size << endl;
		cout << "cgi_path : " << _s[i].cgi_path << endl;
		for (size_t j = 0; j < _s[i].cgi_file_types.size(); j++)
			cout << "cgi_file_types : " << _s[i].cgi_file_types[j] << endl;
		for (size_t j = 0; j < _s[i].location.size(); j++) {
			cout << GREEN "LOCATION : " RESET << endl;
			cout << "location : " << _s[i].location[j].location << endl;
			cout << "download_path : " << _s[i].location[j].download_path << endl;
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








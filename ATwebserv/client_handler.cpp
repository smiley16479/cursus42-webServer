#include "client_handler.hpp"
#include "color.hpp"


client_handler::client_handler(/* args */)
{
}

client_handler::~client_handler()
{
}

bool client_handler::is_request_fulfilled(int client_fd)
{
	// cout << BLUE "DANS IS_REQUEST_FULFILLED, size of current reqst : " << clients[client_fd].rqst.length() <<"\n" RESET;
		
	if (clients[client_fd].rqst.substr(0, 4) == "POST")
		return is_post_rqst_fulfilled(client_fd); // HANDLE RECOGNITION OF ENDED POST RQST

	// for (size_t i = clients[client_fd].rqst.length() - 4; i < clients[client_fd].rqst.length(); ++i)
	// 	cout << "clients[client_fd][i] :[" << clients[client_fd].rqst[i] << "]\n";

	size_t len = clients[client_fd].rqst.size();
	if (len >= 4 && clients[client_fd].rqst.substr(len - 4, len) == "\r\n\r\n") // SUREMENT UNE MAUVAISE FAÇON DE LE FAIRE
		return true ;
	return false ;
}

void client_handler::remove(struct_epoll& _epoll,int i)
{	
	epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_DEL, _epoll._events[i].data.fd, &_epoll._event);
	clients.erase(_epoll._events[i].data.fd);
	if (close(_epoll._events[i].data.fd))
		throw std::runtime_error("CLOSE FAILLED (client_handler::remove)");
}

void client_handler::clear(int client_fd) {	clients[client_fd].rqst.clear();}

void client_handler::rqst_append(int client_fd, char *str) {clients[client_fd].rqst.append(str);}

string client_handler::get_rqst(int client_fd){return clients[client_fd].rqst;}

// si la requete d'un des clients est plus longue a traiter que son time_out (set ds la config) on ferme la connexion ... puis on remove le client
void client_handler::check_all_timeout(struct_epoll& _epoll) 
{	
	for (std::map<int, client_info>::iterator it = clients.begin(); it != clients.end(); it++)
		if (time(NULL) - it->second.rqst_time_start > it->second.time_out) { // COPY DE REMOVE CERTAINEMENT MIEUX A FAIRE...
			cout << "elapsed time : " << time(NULL) - it->second.rqst_time_start <<  "it->second.time_out : " <<  it->second.time_out << endl;
			epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_DEL, it->first, &_epoll._event);
			if (close(it->first)) 
				throw std::runtime_error("CLOSE FAILLED (client_handler::remove)");
			clients.erase(it->first);
		}
}

// Ajoute un client à l'intance e_poll, à la structure client_info et initialize son time_out
void client_handler::add(struct_epoll& _epoll, int time_out, int i)
{
	int client_fd;
	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);
	if ((client_fd = accept4(_epoll._events[i].data.fd, (struct sockaddr *)&clientaddr, &len, SOCK_NONBLOCK)) < 0)
		throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");
	clientaddr.sin_addr;
	//SET NON BLOCK
	int opt = 1;
	setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("fcntl F_SETFL, FNDELAY | FASYNC ");
		exit(EXIT_FAILURE);
	}
	//END
	_epoll._event.events = EPOLLIN | EPOLLOUT | EPOLLET;
	_epoll._event.data.fd = client_fd;
	if(epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_ADD, client_fd, &_epoll._event)) {
		fprintf(stderr, "Failed to add file descriptor to epoll\n");
		// close(_epoll_fd);
		throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}	
	clients[client_fd].time_out = time_out;
	time(&clients[client_fd].rqst_time_start);
}

bool client_handler::is_post_rqst_fulfilled(int client_fd)
{cout << YELLOW "DANS IS_POST_REQUEST_FULFILLED\n";
return true;
/*
	if (clients[client_fd].post_boundary.empty()) {
		size_t bundary_pos;
		if ((bundary_pos = clients[client_fd].rqst.find("boundary")) != string::npos && (bundary_pos += 11)) // +9 == "boundary=".length, moins deux des premiers '-' +2
			clients[client_fd].post_boundary = clients[client_fd].rqst.substr(bundary_pos, clients[client_fd].rqst.find_first_of('\r', bundary_pos) - bundary_pos);
			// cout << "bundary_pos : "<< bundary_pos << ", bundary_pos of fisrt \\r in boundary : " << clients[client_fd].rqst.find_first_of('\r', bundary_pos) << endl;
			// cout << "boundary : [" << clients[client_fd].post_boundary << "]" RESET << endl;
			// cout << "boundary : [" << clients[client_fd].post_boundary + "--\r\n" << "]" RESET << endl;
	}
	if	(clients[client_fd].rqst.rfind(clients[client_fd].post_boundary + "--") != string::npos)
		return true;
	return false;
	*/
}

// POST / HTTP/1.1^M$
// Host: localhost:8081^M$
// User-Agent: curl/7.64.0^M$
// Accept: */*^M$
// Content-Length: 875^M$
// Content-Type: multipart/form-data; boundary=------------------------8a159e385c883f4f^M$
// ^M$
// --------------------------8a159e385c883f4f^M$
// Content-Disposition: form-data; name="text"; filename="color.hpp"^M$
// Content-Type: application/octet-stream^M$
// ^M$

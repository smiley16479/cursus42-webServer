#include "client_handler.hpp"
#include "color.hpp"

client_handler::client_handler(/* args */)
{
	clients.clear();
}

client_handler::~client_handler()
{

}

void client_handler::clear(int client_fd) {	clients[client_fd].rqst.clear();}

void client_handler::rqst_append(int client_fd, char *str, int read_bytes) {clients[client_fd].rqst.append(str, read_bytes);}

string client_handler::get_rqst(int client_fd){return clients[client_fd].rqst;}

// si la requete d'un des clients est plus longue a traiter que son time_out (set ds la config) on ferme la connexion ... puis on remove le client
void client_handler::check_all_timeout() 
{
	size_t	tmp;

	if (!clients.size() || clients.empty())
		return ;
	for (std::vector<client_info>::iterator it = clients.begin(); it < clients.end(); ++it)
	{
		if (it->com_socket == -1)
		{
			tmp = it - clients.begin();
			it->remove();
			clients.erase(it);
			std::cout << "Client erased !" << std::endl;
			it = clients.begin() + tmp;
		}
		else if (time(NULL) - it->rqst_time_start > it->time_out) { // COPY DE REMOVE CERTAINEMENT MIEUX A FAIRE...
//			std::cout << "CLIENT TIMED OUT" << std::endl;
			tmp = it - clients.begin();
			it->remove();
			clients.erase(it);
			std::cout << "Client erased !" << std::endl;
			it = clients.begin() + tmp;
			return ;
		}
		else if (it->mode == RECV || it->mode == CHUNKED)
		{
			it->set_recv_mode();
		}
		else
			it->set_send_mode();
	}
}

// Ajoute un client à l'intance e_poll, à la structure client_info et initialize son time_out

void client_handler::add(struct_epoll& _epoll, int time_out, int i)
{
	client_info	new_client;
	int client_fd;
	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);
	struct epoll_event	ev;
	char	addr_str[INET_ADDRSTRLEN];

	if ((client_fd = accept4(_epoll._events[i].data.fd, (struct sockaddr *)&clientaddr, &len, SOCK_NONBLOCK)) < 0)
		throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");
//	clientaddr.sin_addr;
	bzero(&ev, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = client_fd;
	inet_ntop(AF_INET, &clientaddr, addr_str, INET_ADDRSTRLEN);
	std::cout << "Adding client fd_" << client_fd << " to epoll interest list" << std::endl;
	std::cout << "Client address is : " << addr_str << std::endl;
	if (epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_ADD, client_fd, &ev)) {
		std::cerr << "Failed to add file descriptor to epoll" << std::endl;
		throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}
	new_client.time_out = time_out;
	new_client.loc_fd[0] = -1;
	new_client.loc_fd[1] = -1;
	new_client.cgi_pid = -1;
	new_client.com_socket = client_fd;
	new_client.mode = RECV;
	new_client._epoll = &_epoll;
	time(&new_client.rqst_time_start);
	new_client.rq_mode = NORMAL;
	new_client.chunk_mode = NO_CHUNK;
	new_client.addr = (char*)addr_str;
	clients.push_back(new_client);
}

void	client_handler::fill_resp(int fd, std::string& base)	{
	size_t	pos, end;
	
	pos = base.find("Content-Length");
	if (pos != std::string::npos)
	{
		end = base.substr(pos).find("\r\n");
		base.replace(pos, end, "Connection: keep-alive\r\nTransfer-Encoding: chunked");
	}
	else
	{
		pos = base.find("\r\n\r\n");
		if (pos != std::string::npos)
			base.replace(pos, 4, "\r\nConnection: keep-alive\r\nTransfer-Encoding: chunked\r\n\r\n");
	}
	(*this).clients[fd].resp = base;
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

client_info*	client_handler::get_info(int fd) {
	if (fd == -1)
		return (NULL);
	for (std::vector<client_info>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->com_socket == fd)
		{
			return (&(*it));
		}
		else if (it->loc_fd[0] == fd || it->loc_fd[1] == fd)
		{
			return (&(*it));
		}
	}
	return (NULL);
}

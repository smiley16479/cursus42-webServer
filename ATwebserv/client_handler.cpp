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
			clients.erase(it);
			std::cout << "Client erased !" << std::endl;
			it = clients.begin() + tmp;
		}
		else if (time(NULL) - it->rqst_time_start > it->time_out) { // COPY DE REMOVE CERTAINEMENT MIEUX A FAIRE...
			std::cout << "CLIENT TIMED OUT" << std::endl;
			std::cout << "Client removed from tracked fd !" << std::endl;
			it->remove();
			return ;
		}
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

	if ((client_fd = accept4(_epoll._events[i].data.fd, (struct sockaddr *)&clientaddr, &len, SOCK_NONBLOCK)) < 0)
		throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");
//	clientaddr.sin_addr;
	//SET NON BLOCK
//	int opt = 1;
//	setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
//	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
//	{
//		perror("fcntl F_SETFL, FNDELAY | FASYNC ");
//		exit(EXIT_FAILURE);
//	}
	//END
	bzero(&ev, sizeof(ev));
	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.fd = client_fd;
	if(epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_ADD, client_fd, &ev)) {
		std::cerr << "Failed to add file descriptor to epoll" << std::endl;
		// close(_epoll_fd);
		throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}
	new_client.time_out = time_out;
	new_client.loc_fd = -1;
	new_client.redir_mode = NONE;
	new_client.com_socket = client_fd;
	new_client.mode = RECV;
	new_client._epoll = &_epoll;
	time(&new_client.rqst_time_start);
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

int	client_handler::chunked_rqst(struct_epoll& _epoll, int fd)	{
	int	read_bytes;
	char	str[MAX_LEN];

	(void)_epoll;
	if ((read_bytes = recv(fd, str, sizeof(str), MSG_NOSIGNAL)) != -1)
	{
		this->rqst_append(fd, str, read_bytes);
		this->time_reset(this->clients[fd].time_out, fd);
	}
	else
	{
		return (1);
	}
//	if (this->is_request_fulfilled(fd)) 
//	{
//		return (1);
//	}
	return (0);
}

int	client_handler::chunked_resp(int fd)	{
	std::string	tmp;
	size_t	pos;
	char	buf[20];

	if ((*this).clients[fd].resp.substr(0, 4) == "HTTP")
	{
		pos = this->clients[fd].resp.find("\r\n\r\n");
		tmp.clear();
		if (pos != std::string::npos)
			tmp.append(this->clients[fd].resp.substr(0, pos + 4));
//		std::cout << "chunked header:" << std::endl;
//		std::cout << tmp << std::endl;
		if (send(fd, tmp.c_str(), tmp.length(), MSG_NOSIGNAL) == -1)
		{
	//		perror("Send");
	//		this->clients[fd].resp.clear();
	//		return (1);
		}
		else
		{
			(*this).clients[fd].resp = (*this).clients[fd].resp.substr(pos + 4);
			this->time_reset(this->clients[fd].time_out, fd);
		}
		return (0);
	}
	else if ((*this).clients[fd].resp.length() > MAX_LEN)
	{
		sprintf(buf, "%lx", (*this).clients[fd].resp.substr(0, MAX_LEN).length());
		tmp.clear();
		tmp.append(buf);
		tmp.append("\r\n");
		tmp.append((*this).clients[fd].resp.substr(0, MAX_LEN));
		tmp.append("\r\n");
		if (send(fd, tmp.c_str(), tmp.length(), MSG_NOSIGNAL) == -1)
		{
		//	perror("Send");
		//	this->clients[fd].resp.clear();
		//	return (1);
		}
		else
		{
			(*this).clients[fd].resp = (*this).clients[fd].resp.substr(MAX_LEN);
			this->time_reset(this->clients[fd].time_out, fd);
		}
		return (0);
	}
	else
	{
		sprintf(buf, "%lx", (*this).clients[fd].resp.substr(0, MAX_LEN).length());
		tmp.clear();
		tmp.append(buf);
		tmp.append("\r\n");
		tmp.append((*this).clients[fd].resp);
		if (send(fd, tmp.c_str(), tmp.length(), MSG_NOSIGNAL) == -1)
		{
	//		perror("Send");
	//		this->clients[fd].resp.clear();
	//		return (1);
			return (0);
		}
		tmp.clear();
		tmp.append("0\r\n\r\n");
		this->clear(fd);
		this->clients[fd].resp.clear();
		if (send(fd, tmp.c_str(), tmp.length(), MSG_NOSIGNAL) == -1)
		{
	//		perror("Send");
	//		this->clients[fd].resp.clear();
	//		return (1);
			return (0);
		}
		else
			this->time_reset(this->clients[fd].time_out, fd);
		return (1);
	}
}

int	client_handler::redir_cgi(client_info& client)
{
	int	read_bytes;
	char sd[MAX_LEN];

	(void)client;
	read_bytes = read(client.loc_fd, sd, MAX_LEN);
	client.buf.append(sd, read_bytes);
//	std::cout << client.buf << std::endl;
	if (read_bytes == -1)
	{
		return (0);
	}
	else if (read_bytes < MAX_LEN)
	{
		std::cout << read_bytes << std::endl;
		close(client.loc_fd);
		client.loc_fd = -1;
		client.redir_mode = NONE;
		return (1);
	}
	return (0);
}

int	client_handler::redir_read(client_info& client)
{
	int	read_bytes;
	char sd[MAX_LEN];

	(void)client;
	read_bytes = read(client.loc_fd, sd, MAX_LEN);
	client.buf.append(sd, read_bytes);
//	std::cout << client.buf << std::endl;
	if (read_bytes == -1)
	{
		return (0);
	}
	else if (read_bytes < MAX_LEN)
	{
		close(client.loc_fd);
		client.loc_fd = -1;
		client.redir_mode = NONE;
		return (1);
	}
	return (0);
}

int	client_handler::redir_write(client_info& client)
{
	int	wrote_bytes;
	std::string	tmp;

	if (client.buf.length() > MAX_LEN)
	{
		tmp = client.buf.substr(0, MAX_LEN);
		client.buf = client.buf.substr(MAX_LEN);
		wrote_bytes = write(client.loc_fd, tmp.c_str(), tmp.length());
		if (wrote_bytes == -1)
		{
			std::cout << "error write" << std::endl;
			return (0);
		}
		std::cout << "keep writing !" << std::endl;
		return (0);
	}
	else
	{
		wrote_bytes = write(client.loc_fd, client.buf.c_str(), client.buf.length());
		if (wrote_bytes == -1)
		{
			std::cout << "error write" << std::endl;
			return (0);
		}
		std::cout << "end write" << std::endl;
//		std::cout << client.rqst << std::endl;
		close(client.loc_fd);
		client.loc_fd = -1;
		client.redir_mode = NONE;
		client.buf.clear();
		return (1);
	}
}

std::vector<int>	client_handler::handle_pendings(struct_epoll& _epoll)	{
	std::vector<int>	ret;
	std::vector<client_info>::iterator tmp;

	for (std::vector<client_info>::iterator it = clients.begin(); it != clients.end(); )
	{
		if (it->loc_fd != -1)
		{
			if (it->redir_mode == CGI_OUT)
			{
				if (redir_cgi(*it))
					ret.push_back(it->com_socket);
			}
			else if (it->redir_mode == READ)
			{
				if (redir_read(*it))
					ret.push_back(it->com_socket);
			}
			else
			{
				if (redir_write(*it))
				{
//					this->rearm(_epoll, it->second.time_out, it->first);
					ret.push_back(it->com_socket);
				}
			}
			++it;
		}
		else if (!it->rqst.empty())
		{
			if (chunked_rqst(_epoll, it->com_socket))
				ret.push_back(it->com_socket);
			else
			{
				this->time_reset(it->time_out, it->com_socket);
//				std::cout << "SALUT" << std::endl;
//				this->remove(_epoll, it->first);
//				this->rearm(_epoll, it->second.time_out, it->first);
			}
			++it;
		}
		else if (!it->resp.empty())
		{
		//	printf("\nResolving chunked resp\n");
			if (chunked_resp(it->com_socket))
			{
				tmp = it;
				tmp ++;
				std::cout << "CHUNKED TRANSMISSION END" << std::endl;
				it->remove();
				it = std::vector<client_info>::iterator(get_info(tmp->com_socket));
			}
			else
				++it;
		}
		else
			++it;
	}
	return (ret);
}

void client_handler::time_reset(int time_out, int fd)
{
	if (!clients.empty() && get_info(fd) != NULL)
	{
		clients[fd].time_out = time_out;
		time(&clients[fd].rqst_time_start);
	}
}

int	client_handler::no_chunk(int fd)
{
	if (this->clients[fd].rqst.empty() && this->clients[fd].resp.empty())
		return (1);
	else
		return (0);
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
	for (std::vector<client_info>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->com_socket == fd)
		{
			return (&(*it));
		}
	}
	return (NULL);
}

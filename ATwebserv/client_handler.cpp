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
	cout << BLUE "DANS IS_REQUEST_FULFILLED, size of current reqst : " << clients[client_fd].rqst.length() <<"\n" RESET;
	if (clients[client_fd].rqst.substr(0, 4) == "POST")
		/* return false */; // ADD CODE TO HANDLE RECOGNITION OF ENDED POST RQST // PROBLEM
	for (size_t i = clients[client_fd].rqst.length() - 4; i < clients[client_fd].rqst.length(); ++i)
		cout << "clients[client_fd][i] :[" << clients[client_fd].rqst[i] << "]\n";
	
	cout << clients[client_fd].rqst.back();
	size_t len = clients[client_fd].rqst.size();
	if (len >= 4 && clients[client_fd].rqst.substr(len - 4, len) == "\r\n\r\n")
		return true ;
	return false ;
}

void client_handler::clear(int client_fd){	
	clients[client_fd].rqst.clear();
	/* REFRESH TIME_OUT STILL TO BE IMPLEMENTED */ // PROBLEM
}

void client_handler::remove(struct_epoll& _epoll,int i){	
	epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_DEL, _epoll._events[i].data.fd, &_epoll._event);
	clients.erase(_epoll._events[i].data.fd);
	if (close(_epoll._events[i].data.fd)) 
		throw std::runtime_error("CLOSE FAILLED (client_handler::remove)");
}

void client_handler::rqst_append(int client_fd, char *str){	clients[client_fd].rqst.append(str);}
string client_handler::get_rqst(int client_fd){return clients[client_fd].rqst;}

void client_handler::check_all_timeout(){	
	for (size_t i = 0; i < clients.size(); i++)
	{
		/* code */  // PROBLEM
	}
}

void client_handler::add(struct_epoll& _epoll, int time_out, int i){
	int client_fd;
	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);
	if ((client_fd = accept(_epoll._events[i].data.fd, (struct sockaddr *)&clientaddr, &len)) < 0)
		throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");
	clientaddr.sin_addr;
	_epoll._event.events = EPOLLIN;
	_epoll._event.data.fd = client_fd;
	if(epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_ADD, client_fd, &_epoll._event)) {
		fprintf(stderr, "Failed to add file descriptor to epoll\n");
		// close(_epoll_fd);
		throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}	
	clients[client_fd].time_out = time_out;
}
	
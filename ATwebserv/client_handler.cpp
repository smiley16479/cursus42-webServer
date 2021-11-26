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
	cout << BLUE "DANS IS_REQUEST_FULFILLED, size of current reqst : " << clients[client_fd].length() <<"\n" RESET;
	if (clients[client_fd].substr(0, 4) == "POST")
		/* return false */;
	for (size_t i = clients[client_fd].length() - 4; i < clients[client_fd].length(); ++i)
		cout << "clients[client_fd][i] :[" << clients[client_fd][i] << "]\n";
	
	cout << clients[client_fd].back();
	size_t len = clients[client_fd].size();
	if (len >= 4 && clients[client_fd].substr(len - 4, len) == "\r\n\r\n")
		return true ;
	return false ;
}


void client_handler::remove(int client_fd){	clients.erase(client_fd);}
void client_handler::add(int client_fd){	clients[client_fd];}
void client_handler::rqst_append(int client_fd, char *str){	clients[client_fd].append(str);}
string client_handler::get_rqst(int client_fd){return clients[client_fd];}
void client_handler::check_all_timeout(){	
	for (size_t i = 0; i < clients.size(); i++)
	{
		/* code */
	}
	
}
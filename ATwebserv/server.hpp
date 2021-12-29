#ifndef _SERVER_HPP_ 
#define _SERVER_HPP_
#include <sys/socket.h>	// pour socket()
#include <sys/types.h>	// 
#include <sys/epoll.h>	// for epoll_create1(), epoll_ctl(), struct epoll_event 
#include "color.hpp"
#include "struct_webserv.hpp"

class client_handler;
class request_handler;

class server
{
#define BUF_LEN 1000000

private:
// https://stackoverflow.com/questions/6946217/how-to-access-the-contents-of-a-vector-from-a-pointer-to-the-vector-in-c
    std::vector<server_info> _s;
    struct_epoll _epoll;
 
    char str[MAX_LEN];

public:
    server(std::string);
    ~server();
    void display_server(void); // affiche les variables d(es) server(s) actuels
    void initialize(void); // creer bind et ajoute à epoll le(s) socket(s) de(s) server(s)
    void run(void); // lance les étapes de configuration des sockets initialize(), de run

    /* PRIVATE */
    int is_new_client(int fd);
    int get_time_out(int id_serv);
	void	select_send_method(client_handler&, request_handler&, int fd);
	void	response_handler(client_handler&, request_handler&, int fd);
};

struct epoll_event*	get_event(struct_epoll& _epoll, int fd);

#endif

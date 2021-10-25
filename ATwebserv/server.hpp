#ifndef _SERVER_HPP_ 
#define _SERVER_HPP_ 
#include "color.hpp"
#include "struct_webserv.hpp"

class server
{
private:
    std::vector<server_info> *_si;

public:
    server(std::vector<server_info> *si);
    ~server();
    void run(void);
    void display_server(void);
};

#endif
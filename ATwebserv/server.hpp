#ifndef _SERVER_HPP_ 
#define _SERVER_HPP_ 
#include "color.hpp"
#include "struct_webserv.hpp"

class server
{
private:
// https://stackoverflow.com/questions/6946217/how-to-access-the-contents-of-a-vector-from-a-pointer-to-the-vector-in-c
    std::vector<server_info> *_si;

public:
    server(std::string);
    ~server();
    void run(void);
    void display_server(void);
};

#endif
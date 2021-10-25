#include "server.hpp"
using namespace std;

server::server(std::vector<server_info> *si) : _si(si)
{
}

server::~server()
{
}

void server::run(void) {}

void server::display_server(void)
{
    for (size_t i = 0; i < _si->size(); i++)
    {
        for (size_t j = 0; j < (*_si)[i].server_name.size(); j++)
          cout << "server_name : " << (*_si)[i].server_name[j] << endl;
        cout << "time_out : " << (*_si)[i].time_out << endl;
        cout << "port : " << (*_si)[i].port << endl;
        cout << "host : " << (*_si)[i].host << endl;
        cout << "error_page : " << (*_si)[i].error_page << endl;
        cout << "max_file_size : " << (*_si)[i].max_file_size << endl;
    }
    
}

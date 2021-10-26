#include "server.hpp"
#include "config_checker.hpp"

using namespace std;

server::server(std::string av)
{
  config_checker confCheck;
	confCheck.check_conFile(av);
  this->_si = confCheck._si;
}

server::~server()
{
	cout << RED "server destructeur..." RESET << endl;
  delete _si;
}

void server::run(void) {
  for (size_t i = 0; i < _si->size(); i++)
    if (0); // on en est a connecter les server au reseau
}

void server::display_server(void)
{
  for (size_t i = 0; i < _si->size(); i++)
  {
    cout << GREEN "SERVER : " RESET << endl;
    for (size_t j = 0; j < (*_si)[i].server_name.size(); j++)
      cout << "server_name : " << (*_si)[i].server_name[j] << endl;
    cout << "time_out : " << (*_si)[i].time_out << endl;
    cout << "port : " << (*_si)[i].port << endl;
    cout << "host : " << (*_si)[i].host << endl;
    cout << "error_page : " << (*_si)[i].error_page << endl;
    cout << "max_file_size : " << (*_si)[i].max_file_size << endl;
    for (size_t j = 0; j < (*_si)[i].cgi_file_types.size(); j++)
      cout << "cgi_file_types : " << (*_si)[i].cgi_file_types[j] << endl;
    for (size_t j = 0; j < (*_si)[i].location.size(); j++) {
      cout << GREEN "LOCATION : " RESET << endl;
      cout << "location : " << (*_si)[i].location[j].location << endl;
      cout << "auth_basic : " << (*_si)[i].location[j].auth_basic << endl;
      cout << "auth_user_file : " << (*_si)[i].location[j].auth_user_file << endl;
      cout << "autoindex : " << (*_si)[i].location[j].autoindex << endl;
      cout << "index : " << (*_si)[i].location[j].index << endl;
      cout << "max_file_size : " << (*_si)[i].location[j].max_file_size << endl;
      cout << "return_directive : " << (*_si)[i].location[j].return_directive << endl;
      cout << "root : " << (*_si)[i].location[j].root << endl;
      cout << "allowed_method : ";
      for (size_t k = 0; k < (*_si)[i].location[j].allowed_method.size(); k++)
        cout << (*_si)[i].location[j].allowed_method[k] << (k < (*_si)[i].location[j].allowed_method.size() - 1 ? ", " : "");
      cout << endl << "return : ";
      for (size_t k = 0; k < (*_si)[i].location[j].retour.size(); k++)
        cout << (*_si)[i].location[j].retour[k] << (k < (*_si)[i].location[j].retour.size() - 1 ? ", " : "");
      cout << endl;
    }
  }
}








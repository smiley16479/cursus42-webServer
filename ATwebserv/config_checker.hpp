#ifndef _CONFIG_CHECKER_HPP_
#define _CONFIG_CHECKER_HPP_
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
#include "color.hpp"
#include "struct_webserv.hpp"
using namespace std;

class config_checker
{
	/* data */
	class configException: public std::exception
	{
	  public :
	  configException(): _str("Config File : error.") {};
	  configException(std::string str) : _str(str + " Config File : error.") {};
	  ~configException() throw() {};
	  virtual const char* what() const throw()
	  {
	    return  _str.c_str();
	  }
	  private :
	  std::string _str;
	} myex;

/* 
**		TYPES
*/
    typedef     bool (config_checker::*configure)(const std::string&, const std::string&);
/* 
**		ATTRIBUTS
*/
	server_info _si;
	map<string, vector<string> > _semantic;
	
public:

	config_checker(/* args */);
	~config_checker();

// le throw spécifier doit throw le type exact sinon std::unexpected is called and the catch block doesn't work # https://openclassrooms.com/fr/courses/1894236-programmez-avec-le-langage-c/1903837-gerez-des-erreurs-avec-les-exceptions#/id/r-1903804
	void check_conFile(std::string); // throw (/* Si on mets rien ça throw rien */) {throw (configException());}; 

private:
/* 
**		PRIVATE FUNCTION (UTILITAIRES)
*/
	void check_serv_part(std::ifstream&);
	void check_loca_part(std::ifstream&);
	bool is_not_allowed(string key,string val);
	void string_vector_insert(ifstream& ifs, string& where_to_insert);
	void valid_port(std::ifstream& ifs);
	void valid_server_nm(std::ifstream& ifs);
	void valid_error_page(std::ifstream& ifs);

};

#endif
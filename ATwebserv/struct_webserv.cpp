// #ifndef _STRUCT_WEBSERV_HPP_
// #define _STRUCT_WEBSERV_HPP_
#include <string>
#include <iostream>
#include <vector>

typedef struct struct_test {
	int a = 1;
	int b = 2;
	int c = 3;
} t_test;



struct struct_info {
	bool uniq;
	std::string	val; // Valeur attachée à la clé (string) de la map
	typedef union u_info {
		int un = 1;
		int deux;
		int trois;
	} t_info;
	typedef struct my_struct {
		int un = 1;
		int deux = 2;
		int trois = 3;
	} t_my_struct;
	typedef enum u_enum {
		un, deux, trois
	} t_enum;
};

typedef struct server_info {
	std::string port;
	std::string host;
	std::vector<std::string> server_name;
	std::string error_page;
	std::string time_out;
	std::string cgi_file_types;
	std::string location;
} t_server_info;




int main(int argc, char const *argv[])
{
	struct struct_info info;
	t_test test;
	// std::cout << info.
	std::cout << test.c;

	return 0;
}


/* 
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}
 */

// #endif
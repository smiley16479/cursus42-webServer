#ifndef _STRUCT_WEBSERV_HPP_
#define _STRUCT_WEBSERV_HPP_
#include <string>
#include <iostream>
#include <vector>

struct server_info {
	std::string port;
	std::string host;
	std::vector<std::string> server_name;
	std::string error_page;
	std::string time_out;
	std::string cgi_file_types;
	std::string location;
};

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

#endif
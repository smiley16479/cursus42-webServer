#include <map>
#include <string>
#include <iostream>

using namespace std;

class client_handler
{
private:
	map <int, string> clients;

public:
	client_handler(/* args */);
	~client_handler();
	bool is_request_fulfilled(int);
	void remove(int); // REMOVE A CLIENT
	void add(int); // ADD A CLIENT
	void rqst_append(int , char *); // APPEND DIFFERENT REQUEST CHUNK
	string get_rqst(int); // RETURN FINAL REQUEST
	void check_all_timeout(void);

};



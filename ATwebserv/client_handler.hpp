#include <map>


using namespace std;

class client_handler
{
// private:
public: // a mettre private (peut-être) quand j'aurai fait les setter et getter...
	map <int, string> clients;

public:
	client_handler(/* args */);
	~client_handler();
};



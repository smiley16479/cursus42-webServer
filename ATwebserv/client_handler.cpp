#include "client_handler.hpp"
#include "color.hpp"


client_handler::client_handler(/* args */)
{
}

client_handler::~client_handler()
{
}

bool client_handler::is_request_fulfilled(int client_fd)
{
	cout << BLUE "DANS IS_REQUEST_FULFILLED, size of current reqst : " << clients[client_fd].rqst.length() <<"\n" RESET;
	client_info& client = clients[client_fd];
	size_t len = client.rqst.size();

	if (client.request_fulfilled)
		return true;
	else if (client.rqst.substr(0, 4) == "POST")
		return (client.request_fulfilled = is_POST_request_fulfilled(client));
	else if (client.rqst.substr(0, 3) == "PUT")
		return (client.request_fulfilled = is_PUT_request_fulfilled(client));
	else if (len >= 4 && client.rqst.substr(len - 4, len) == "\r\n\r\n") // SUREMENT UNE MAUVAISE FAÇON DE LE FAIRE
		return (client.request_fulfilled = true) ;
	return (client.request_fulfilled = false) ;
}

/* 
struct client_info {
	std::string	resp; //Ajout Arthur
	std::string rqst;
	std::string post_boundary; //Ajout Arthur
	time_t		rqst_time_start;
	size_t		_cLen; //Ajout Arthur
	int			time_out;
};
 */

bool client_handler::is_POST_request_fulfilled(client_info& client)
{//https://datatracker.ietf.org/doc/html/rfc7231#section-4.3.3
	cout << BLUE "DANS is_POST_request_fulfilled, size of current reqst : " << client.rqst.length() << "\n" RESET;

	size_t pos1, pos2;
	// Si NONE, identifie le type de post requete concernée (POST_MULTIPART / POST_URL_ENCODED / POST_CHUNCK):
	if (client.rqst_type == NONE) {
		cout << YELLOW "NONE\n" RESET;
		if ( (pos1 = client.rqst.find("\r\n\r\n")) == string::npos ) // Vérifie qu'on a au moins les headers
			return false;
		// PROBLEM SI ON A PAS L'UN DE CES DEUX TYPE DE HEADER ET QUE LA REQUETE EST LONGUE ON VA FREEZE LE TEMPS DE LIRE TOUTE LA REQST => resolu grace à portion_search()
		// else if ( (pos2 = client.rqst.find("Content-Length:")) != string::npos && pos2 < pos1 ) {
		else if ( (pos1 += 4) && (pos2 = portion_search(client.rqst, "Content-Length:", 0, pos1)) != string::npos && pos2 < pos1 ) {
			pos2 += 15; // 15 == "Content-Length:"
			client.clen = strtol(&client.rqst[pos2], NULL, 10); // Stockage de Content-Length ds le client_info
			if ( (pos2 = portion_search(client.rqst, "multipart/form-data", 0, pos1)) != string::npos && pos2 < pos1 ) {
				// ON CHOPE LA BOUNDARY : boundary=[------------------------772ed66a82c8bebbM] (QUE C QU'IL Y A ENTRE LES CROCHETS)
				size_t limit_end  = client.rqst.find_first_of("\r\n", client.rqst.find("boundary=--------", pos2));
				size_t limit_beg  = client.rqst.find_last_of('=', limit_end) + 1;
				if (limit_end == string::npos || limit_beg == string::npos)
					return false;
				// Stockage de la boundary de la requete post ds la struct client_info (client.post_boundary)
				cout << RED "Post_boundary : " RESET << (client.post_boundary = client.rqst.substr(limit_beg, limit_end - limit_beg)) << endl;
				client.rqst_type = POST_MULTIPART;
			}
			else if ( (pos2 = portion_search(client.rqst, "application/x-www-form-urlencoded", 0, pos1)) != string::npos && pos2 < pos1 )
				client.rqst_type = POST_URL_ENCODED;
		}
		// else if ( (pos2 = client.rqst.find("Transfer-Encoding:")) != string::npos && pos2 < pos1 ) {
		else if ( (pos2 = portion_search(client.rqst, "Transfer-Encoding:", 0, pos1)) != string::npos && pos2 < pos1 ) {
			// if ( (pos2 = client.rqst.find("chunked")) != string::npos && pos2 < pos1 )
			if ( (pos2 = portion_search(client.rqst, "chunked", 0, pos1)) != string::npos && pos2 < pos1 )
				client.rqst_type = POST_CHUNCK;
		}
		else
			return false;
	}

	// ON regarde si le type de la requete est complète :
	if (client.rqst_type == POST_MULTIPART) {
	// SI ON EST A LA FIN ON DEVRAIT AVOIR LE DELIMITEUR AV "--" EN PREFIXE & SUFIXE : on prends la fin de la requete...
		cout << YELLOW "POST_MULTIPART : \n" RESET;
		cout << client.rqst << endl;
		if (client.rqst.substr(client.rqst.size() - 46).find("--" + client.post_boundary + "--") != string::npos
			&& client.rqst.size() - pos1 >= client.clen )
			return true;
	}
	else if (client.rqst_type == POST_URL_ENCODED) {
		cout << YELLOW "POST_URL_ENCODED : " << client.rqst.size() - pos1 << " client.clen : " << client.clen << "\n" RESET;
		if (client.rqst.size() - pos1 >= client.clen)
			return true;
	}
	else if (client.rqst_type == POST_CHUNCK) {	
	// SI ON EST A LA FIN ON DEVRAIT AVOIR LE "0\r\n\r\n" du chunck de fin : on prends la fin de la requete...
		cout << YELLOW "POST_CHUNCK :\n" RESET;
		cout << client.rqst << endl;
		if (client.rqst.find("0\r\n\r\n", client.rqst.size() - 5) != string::npos)
			return true;
	}
	else
		cout << RED "WTF?\n" RESET;
	return false;
}

bool client_handler::is_PUT_request_fulfilled(client_info& client)
{
		cout << BLUE "DANS is_PUT_request_fulfilled, size of current reqst : " << client.rqst.length() << "\n" RESET;

	size_t pos1, pos2;
	if ( (pos1 = client.rqst.find("\r\n\r\n")) == string::npos ) // Vérifie qu'on a au moins les headers
		return false;
// PROBLEM SI ON A PAS L'UN DE CES DEUX TYPE DE HEADER ET QUE LA REQUETE EST LONGUE ON VA FREEZE LE TEMPS DE LIRE TOUTE LA REQST => resolu grace à portion_search()
// else if ( (pos2 = client.rqst.find("Content-Length:")) != string::npos && pos2 < pos1 ) {
	else if ( (pos2 = portion_search(client.rqst, "Content-Length:", 0, pos1)) != string::npos && pos2 < pos1 ) {
		pos1 += 4; // "\r\n\r\n" == 4
		pos2 += 15; // 15 == "Content-Length:"
		client.clen = strtol(&client.rqst[pos2], NULL, 10);
		cout << "client.clen : " << client.clen << endl;
		cout << MAGENTA "ICI\n" RESET << "client.rqst.size() / pos_boundary : " << client.rqst.size() << " / " << pos1 <<endl;
		cout << MAGENTA "client.rqst : " RESET << client.rqst << endl;
		if ( (client.rqst.size() - pos1) >= client.clen )
			return true;
	}
	return false;
}

void client_handler::remove(struct_epoll& _epoll,int i)
{	
	epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_DEL, _epoll._events[i].data.fd, &_epoll._event);
	clients.erase(_epoll._events[i].data.fd);
	if (close(_epoll._events[i].data.fd)) 
		throw std::runtime_error("CLOSE FAILLED (client_handler::remove)");
}

void client_handler::clear(int client_fd) {	clients[client_fd].rqst.clear(); }
void client_handler::rqst_append(int client_fd, char *str, int byte_recved) {/* cout << strlen(str) << endl; */ clients[client_fd].rqst.append(str, byte_recved); }
client_info& client_handler::get_rqst(int client_fd) { return clients[client_fd]; }

// si la requete d'un des clients est plus longue a traiter que son time_out (set ds la config) on ferme la connexion ... puis on remove le client
void client_handler::check_all_timeout(struct_epoll& _epoll) 
{	
	for (std::map<int, client_info>::iterator it = clients.begin(); it != clients.end(); it++)
		if (time(NULL) - it->second.rqst_time_start > it->second.time_out) { // COPY DE REMOVE CERTAINEMENT MIEUX A FAIRE...
			cout << "elapsed time : " << time(NULL) - it->second.rqst_time_start <<  ", time_out du client : " <<  it->second.time_out << endl;
			epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_DEL, it->first, &_epoll._event);
			if (close(it->first)) 
				throw std::runtime_error("CLOSE FAILLED (client_handler::remove)");
			clients.erase(it->first);
		}
}

// Ajoute un client à l'intance e_poll, à la structure client_info et initialize son time_out
void client_handler::add(struct_epoll& _epoll, int time_out, int i)
{
	int client_fd;
	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);
	if ((client_fd = accept(_epoll._events[i].data.fd, (struct sockaddr *)&clientaddr, &len)) < 0)
		throw std::runtime_error("ERROR IN SOCKET ATTRIBUTION");
	// clientaddr.sin_addr;
	_epoll._event.events = EPOLLIN;
	_epoll._event.data.fd = client_fd;
	if(epoll_ctl(_epoll._epoll_fd, EPOLL_CTL_ADD, client_fd, &_epoll._event)) {
		fprintf(stderr, "Failed to add file descriptor to epoll\n");
		// close(_epoll_fd);
		throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}	
	clients[client_fd].time_out = time_out;
	time(&clients[client_fd].rqst_time_start);
}

//Return the position of needle found in haystack otherwise return npos
//Beware that haystack must be bigger than needle and its boundary accurate, otherwise 😕🤮
size_t client_handler::portion_search(string haystack, string needle, size_t from, size_t to)
{
	string::iterator first  = haystack.begin() + from,
					 last   = haystack.begin() + (to > haystack.size() ? haystack.size() : to),
					 s_last = needle.end();
	while (1) {
		string::iterator it = first;
		for (string::iterator s_it = needle.begin(); ; ++it, ++s_it) {
			if (s_it == s_last) return first - haystack.begin();
			if (it == last) return string::npos;
			if (!(*it == *s_it)) break;
		}
		++first;
	}
}


/* POST / HTTP/1.1
Host: 127.0.0.1:8080
 */


/* POST / HTTP/1.1
Host: 127.0.0.1:8080
User-Agent: curl/7.64.0
Accept: +/+
Content-Length: 674
Content-Type: multipart/form-data; boundary=------------------------772ed66a82c8bebbM
	
--------------------------772ed66a82c8bebbM
Content-Disposition: form-data; name="image"; filename="index2.html"M
Content-Type: text/htmlM
M
<!doctype html>
<html>
  <head>
	<link rel="icon" 
	  type="image/png" 
	  href="favicon.ico">
	<title>This is the title of the webpage!</title>
  </head>
  <body>
	<p>This is the index<strong>2</strong>.html</p>
	<p>With a picture <img src="favicon.ico" alt="Tako" style="margin: auto;">.</p>
	<div style="margin: auto;">
	  <img src="favicon.ico" alt="Tako" >
	</div>
	<!-- <meta http-equiv="refresh" content="5; URL=./Gas.mp4" /> -->
  </body>
</html>
M
--------------------------772ed66a82c8bebb--M
*/

/*
//
POST / HTTP/1.1M
Host: 127.0.0.1:8080M
User-Agent: curl/7.64.0M
Accept: +/+M
Content-Length: 674M
Content-Type: multipart/form-data; boundary=------------------------72860daf576785aeM
M
--------------------------72860daf576785aeM */


// MULTIPLE FILE ENVOYÉES !!
// curl -X POST -F image=@files/index2.html -F image=@files/index.html 127.0.0.1:8080

/* 
M
--------------------------f3547eb9f0e8057eM
Content-Disposition: form-data; name="image"; filename="index2.html"M
Content-Type: text/htmlM
M
<!doctype html>
<html>
  <head>
	<link rel="icon" 
	  type="image/png" 
	  href="favicon.ico">
	<title>This is the title of the webpage!</title>
  </head>
  <body>
	<p>This is the index<strong>2</strong>.html</p>
	<p>With a picture <img src="favicon.ico" alt="Tako" style="margin: auto;">.</p>
	<div style="margin: auto;">
	  <img src="favicon.ico" alt="Tako" >
	</div>
	<!-- <meta http-equiv="refresh" content="5; URL=./Gas.mp4" /> -->
  </body>
</html>
M
--------------------------f3547eb9f0e8057eM
Content-Disposition: form-data; name="image"; filename="index.html"M
Content-Type: text/htmlM
M
<!doctype html>
<html>
  <head>
	<link rel="icon" 
	  type="image/png" 
	  href="favicon.ico">
	<title>This is the title of the webpage!</title>
  </head>
  <body>
	<p>This is the index.html</p>
	<p>With a picture <img src="favicon.ico" alt="Tako" style="margin: auto;">.</p>
	<div style="margin: auto;">
	  <img src="favicon.ico" alt="Tako" >
	</div>
	<!-- <meta http-equiv="refresh" content="5; URL=./Gas.mp4" /> -->
  </body>
</html>
--------------------------f3547eb9f0e8057e--M
 */


/* 
POST / HTTP/1.1M
Host: 127.0.0.1:8080M
User-Agent: curl/7.64.0M
Accept: +/+M
Content-Length: 674M
Content-Type: multipart/form-data; boundary=------------------------6b0875ece474b327M
M
--------------------------6b0875ece474b327M
Content-Disposition: form-data; name="image"; filename="index2.html"M
Content-Type: text/htmlM
M
<!doctype html>
<html>
  <head>
	<link rel="icon" 
	  type="image/png" 
	  href="favicon.ico">
	<title>This is the title of the webpage!</title>
  </head>
  <body>
	<p>This is the index<strong>2</strong>.html</p>
	<p>With a picture <img src="favicon.ico" alt="Tako" style="margin: auto;">.</p>
	<div style="margin: auto;">
	  <img src="favicon.ico" alt="Tako" >
	</div>
	<!-- <meta http-equiv="refresh" content="5; URL=./Gas.mp4" /> -->
  </body>
</html>
M
--------------------------6b0875ece474b327--M 
*/

/* 
	cout << "name [" << name + "]" << endl;
	if ( (pos = clients[client_fd].rqst.find(name, pos_boundary)) != string::npos )
		if ( (pos1 = clients[client_fd].rqst.find_first_of('"', pos + name.size())) != string::npos )
			name = clients[client_fd].rqst.substr(pos + name.size(), pos1 - (pos + name.size()) );
	cout << "pos " << pos << " pos1 " << pos1 <<  " name [" << name + "]" << endl;
 */
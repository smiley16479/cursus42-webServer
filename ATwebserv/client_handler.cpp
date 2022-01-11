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
	if (clients[client_fd].rqst.substr(0, 4) == "POST")
		return is_POST_request_fulfilled(client_fd); // ADD CODE TO HANDLE RECOGNITION OF ENDED POST RQST // PROBLEM
	// for (size_t i = clients[client_fd].rqst.length() - 4; i < clients[client_fd].rqst.length(); ++i)
	// 	cout << "clients[client_fd][i] :[" << clients[client_fd].rqst[i] << "]\n";
	// cout << clients[client_fd].rqst.back();
	
	size_t len = clients[client_fd].rqst.size();
	if (len >= 4 && clients[client_fd].rqst.substr(len - 4, len) == "\r\n\r\n") // SUREMENT UNE MAUVAISE FAÇON DE LE FAIRE
		return true ;
	return false ;
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

bool client_handler::is_POST_request_fulfilled(int client_fd)
{//https://datatracker.ietf.org/doc/html/rfc7231#section-4.3.3
	cout << BLUE "DANS is_POST_request_fulfilled, size of current reqst : " << clients[client_fd].rqst.length() << "\n" RESET;

	switch (clients[client_fd].rqst_type)
	{
	case NONE: // Si NONE, identifie le type de post requete concernée :
		cout << YELLOW "HELLO :) rqst_type tjrs NONE(0)? " << clients[client_fd].rqst_type << "\n" RESET;
		if (clients[client_fd].rqst.find("\r\n\r\n") == string::npos)
			return false;
		// PROBLEM SI ON A PAS L'UN DE CES DEUX TYPE DE HEADER ET QUE LA REQUETE EST LONGUE ON VA FREEZE LE TEMPS DE LIRE TOUTE LA REQST
		else if (clients[client_fd].rqst.find("Content-Length:") != string::npos)
			clients[client_fd].rqst_type = POST_REG;
		else if (clients[client_fd].rqst.find("Transfer-Encoding:") != string::npos) {
			if (clients[client_fd].rqst.find("chunked") != string::npos)
				clients[client_fd].rqst_type = POST_CHUNCK;
		}
		else
			return false;
	// Traitement du type précédement identifié :
	case POST_REG:
		return is_POST_regular_fulfilled(client_fd);
	case POST_CHUNCK:
		return is_POST_chunk_fulfilled(client_fd);
	default:
		cout << RED "WTF?\n" RESET;
		break;
	}
	return false;
}

bool client_handler::is_POST_chunk_fulfilled(int client_fd)
{
	return false;
}

bool client_handler::is_POST_regular_fulfilled(int client_fd)
{
	size_t body_size;
	size_t pos, pos1, pos_boundary, pos_last_boundary;
	// PROBLEM ON NE FAIT PAS LE MULTI FILES TRANSFERT LÀ !
	// RECHERCHE DU HEADER DE LA TAILLE DU BODY (Content-Length)
	// SI 'Content-Length:' N'EST PAS LÀ, LE HEADER Transfer-Encoding LE REMPLACERA À PRIORI (ou est-ce "Content-Type:")
	if ((pos = clients[client_fd].rqst.find("Content-Length:")) == string::npos)
		return false;
	body_size =  atoi (&clients[client_fd].rqst[pos + 15]); // (+15 == "Content-Length:")
	// RECHERCHE DU COMMENCEMENT DE LA TRANSMISSION DU BODY ( ici --> "\r\n\r\n------------------------772ed66a82c8bebb")
	if ((pos_boundary = clients[client_fd].rqst.find("\r\n\r\n")) == string::npos)
		return false;
	pos_boundary += 4; // (+4 == "\r\n\r\n")
#ifdef _debug_
	cout << "commencement du body (pos_boundary) : " << pos_boundary << endl;
	cout << RED "rqst.size() : " RESET << clients[client_fd].rqst.size() << endl;
	cout << "body_size : " << body_size<< endl;
#endif
	// SI LA TAILLE DU BODY N'EST PAS COMPLETE
	if ( (clients[client_fd].rqst.size() - pos_boundary) < body_size )
		return false;

// ON CHOPE LA BOUNDARY : boundary=[------------------------772ed66a82c8bebbM] (QUE C QU'IL Y A ENTRE LES CROCHETS)
	pos1 = clients[client_fd].rqst.find_first_of("\r\n", clients[client_fd].rqst.find("boundary=--------"));
	pos  = clients[client_fd].rqst.find_last_of('=', pos1) + 1;
	// Stockage de la boundary de la requete post ds la struct client_info (clients[client_fd].post_boundary)
	cout << RED "1 : " RESET << (clients[client_fd].post_boundary = clients[client_fd].rqst.substr(pos, pos1 - pos)) << endl;

	// SI ON EST A LA FIN ON DEVRAIT AVOIR LE DELIMITEUR AV "--" EN PREFIXE & SUFIXE : on prends la fin de la requete...
	if (clients[client_fd].rqst.substr(clients[client_fd].rqst.size() - 46).find("--" + clients[client_fd].post_boundary + "--") == string::npos)
		return false;
	cout << "END POST BOUNDARY FOUND : [" << clients[client_fd].rqst.substr(clients[client_fd].rqst.size() - 46) << "]\n";
	cout << RED "2 : " RESET << (pos_last_boundary = clients[client_fd].rqst.find_last_of("\r\n", clients[client_fd].rqst.size() - 4)) << endl;

// GET NAME AND FILENAME INSIDE BOUNDARY
	string name("name=\"");
	string filename("filename=\"");
	// cout << "name [" << name + "]" << endl;
	if ( (pos = clients[client_fd].rqst.find(name, pos_boundary)) != string::npos )
		if ( (pos1 = clients[client_fd].rqst.find_first_of('"', pos + name.size())) != string::npos )
			name = clients[client_fd].rqst.substr(pos + name.size(), pos1 - (pos + name.size()));
	// cout << "pos " << pos << " pos1 " << pos1 <<  " name [" << name + "]" << endl;

	// cout << "filename [" << filename + "]" << endl;
	if ( (pos = clients[client_fd].rqst.find(filename, pos_boundary)) != string::npos )
		if ( (pos1 = clients[client_fd].rqst.find_first_of('"', pos + filename.size())) != string::npos )
			filename = clients[client_fd].rqst.substr(pos + filename.size(), pos1 - (pos + filename.size()));
	// cout << "pos " << pos << " pos1 " << pos1 <<  " filename [" << filename + "]" << endl;
// END GET NAME AND FILENAME INSIDE BOUNDARY

	ofstream my_file(name + "_transfer");
	if ((pos = clients[client_fd].rqst.find("\r\n\r\n", pos1)) != string::npos && (pos += 4)) // +=4 == "\r\n\r\n"
		if (my_file.is_open())
			my_file << clients[client_fd].rqst.substr(pos, pos_last_boundary - pos - 1);
	
	return true ;
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
	clientaddr.sin_addr;
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
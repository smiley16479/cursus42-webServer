/* 
*      HTTP-message   = start-line
*                      *( header-field CRLF )
*                      CRLF
*                      [ message-body ]
*
*      start-line     = request-line (client)/ status-line (server)

		http://127.0.0.1:8080/ <- c'est ca qui fait bugger

		// https://everything.curl.dev/http/post/chunked  // Site web pour curl
		curl --resolve test_server_block.com:9090:127.0.0.1 http://test_server_block.com:9090/ -X GET // Change le server_name
		curl -X POST -F 'create_to_erase=@Makefile' 127.0.0.1:8081/(create_to_erase) -v // Fait une post request
		curl -T Makefile 127.0.0.1:8080/new/resource/file // Veritable put selon Site web pour curl
		curl -X PUT -F 'file=@compil.sh' 127.0.0.1:8080/create_to_erase -v
		curl -H "Transfer-Encoding: chunked" -d @file http://example.com

*/


#include "request_handler.hpp"
#include "cgi_handler.hpp"

request_handler::request_handler(std::vector<server_info>& server_info) : _si(server_info)
{// BON EN FAIT IL SEMBLE QUE LA SECU NE SOIT PAS LE POINT IMPORTANT DE WEBSERV...
/* const char *array[] = {	"GET", "Host:", "User-Agent:", "Accept:", "Accept-Language:",
												"Accept-Encoding:", "Connection:", "Upgrade-Insecure-Requests:",
												"Cache-Control:", "DNT:", "rol:"};

	for (size_t i = 0; i < sizeof(array) / sizeof(const char*); ++i)
		_hrx[array[i]];
		// _hrx.insert(std::make_pair(array[i], vector<string>()));
#ifdef _debug_
	for (map< string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it)
		cout <<  YELLOW "map it.first : " RESET << it->first << " size() : " << it->second.size() << endl; 
#endif
*/
// CREER LA LOOKUP TABLE
	// _tab = {	
	_tab[0] = &request_handler::handle_get_rqst;
	_tab[1]	= &request_handler::handle_put_rqst;
	_tab[2]	= &request_handler::handle_post_rqst;
		// &request_handler::extract_multi_rqst_body,
		// &request_handler::extract_xform_rqst_body,
		// &request_handler::extract_chunk_rqst_body,
	// };


// REMPLI LA MAP _STATUS POUR LA STATUS LIGNE (TX) HTML
	ifstream fs("configuration_files/HTML_status_msg.txt");
	if (!fs.is_open()) 
		throw (std::runtime_error("Unkown file : configuration_files/HTML_error_msg.txt"));

	string buf_1, buf_2;
	while (std::getline(fs, buf_1)) {
		if (buf_1[0] == '#' || buf_1[0] == '\0')
			continue ;
		std::stringstream ss_2(buf_1);
		ss_2 >> buf_1; 
		std::getline(ss_2, buf_2);
		_status[buf_1].append(buf_2.substr(0, buf_2.find_first_of('\t')));
	}
#ifdef _debug_
	// for (map< string, string>::iterator it = _status.begin(), end = _status.end(); it != end; ++it)
	// cout <<  YELLOW "map it.first : [" RESET << it->first << "] second : [" << it->second << "]" << endl;
#endif
}

request_handler::~request_handler()
{
}

/* cout << distance(mymap.begin(),mymap.find("198765432")); */ // <- Get index of the pair(key_type, mapped_type) TIPS&TRICKS

void request_handler::reader(client_info& cl_info)
{
	string buf_1, buf_2;
	_c = &cl_info;
	std::stringstream ss_1(cl_info.rqst);
	cout << RED "DANS HEADER READER" RESET "\n" << endl; //cl_info.rqst << endl;
	// LECTURE DE LA START_LINE
	if (std::getline(ss_1, buf_1)) {
		std::stringstream ss_2(buf_1);
		while (ss_2 >> buf_1)
			_hrx["A"].push_back(buf_1);
	}

	for (auto it = _hrx["A"].begin(); it != _hrx["A"].end(); it++)
		cout << "*it : " << *it << endl;

	// LECTURE DU RESTE DE LA REQUETE
	while (std::getline(ss_1, buf_1)) {
		if (buf_1[0] == '\r') { 
			// 3eme SOLUTION : SI C'EST UNE REQUESTE POST ON IGNORE LE BODY POUR USAGE ULTÉRIEUR AV _c (on utilise celle la pour le moment)
			break ;
			// 2eme SOLUTION :
		// C'EST ICI QU'ON DOIT UTILISER extract_post_rqst_body peut être utiliser slmt des position poour ne pas faire de copie ou
			// extract_post_rqst_body(cl_info);
			// 1ere SOLUTION :
			// while (std::getline(ss_1, buf_1))
			// 	_hrx["BODY"].push_back(buf_1);
			// cout << "BODY size : " << _hrx["BODY"].size() << endl;
		}
		std::stringstream ss_2(buf_1);
		while (ss_2 >> buf_2) {
			// if (_hrx.find(buf_2) != _hrx.end()) { // VERIFICATION DE LA SYNTAXE DES HEADERS AU CAS OU L'ON VEUILLE FAIRE DE LA SÉCURITÉ...
				string index = buf_2;
				while (ss_2 >> buf_2)
					_hrx[index].push_back(buf_2);
		}
			// }
			// else
			// 	throw (std::runtime_error( "Unkown header field (header_reader) : " + buf_2));
#ifdef _debug_
		// cout << RED << "buf_1.size : "<< buf_1.size() << RESET "[" << buf_1 << "]" << endl;
		// cout << GREEN "ss_2 >> buf_2 : " RESET << "[" << buf_2 << "]" << endl;
		// cout << MAGENTA << "tour" << RESET << endl;
#endif
	}
	set_server_id();
	// cout << RED "APRES GETLINE : " << buf_1 << RESET << endl;
	// this->display();
}

void request_handler::writer(void) {

// PAR DEFAULT ON CONSIDÈRE QUE TOUT SE PASSE BIEN ON CHANGE PAR LA SUITE LE STATUS SI UNE EXCEPTION ARRIVE
	gen_startLine( _status.find("200") );
	gen_date();
	gen_serv();

// EN CAS DE MÉTHODE NON AUTORISÉE DS CETTE LOCATION
	if (resolve_path()) {
		file_type();
		add_all_field();
		add_body(false);
	}
	else if ((ext_id = is_cgi(_hrx["A"], _si[_s_id].cgi_file_types) != -1))
		handle_cgi();
	else if (_hrx["A"][0] == "GET")
		handle_get_rqst();
	else if (_hrx["A"][0] == "PUT")
		handle_put_rqst();
	else if (_hrx["A"][0] == "POST")
		handle_post_rqst();
	else if (_hrx["A"][0] == "HEAD") {
		cout << "Facultatif HEAD method not implemented yet\n";
		file_type();
		gen_CType();
		gen_CLength(2);
		add_all_field();
		// gen_startLine( _status.find("405") ); // 405 Not allowed - 403 Forbidden
	}
	else if (_hrx["A"][0] == "DELETE") {
		// IF NOT ALLOWED -> 405

		if (_hrx["A"][1][0] == '/')
			_hrx["A"][1].insert(0, ".");
		if( remove( _hrx["A"][1].c_str() ) != 0 ) {
			perror( _hrx["A"][1].c_str() );
			gen_startLine( _status.find("404") ); //  IF NOT FOUND -> 404
		}
		else
			puts( "File successfully deleted" );
	}
	// gen_CType(string());
	// gen_CLength();
	// add_all_field(); 
	// add_body();
	_hrx.clear();
	_htx.clear();
}

	/* FONCTION UNITAIRES DES METHODES PRINCIPALES */

void	request_handler::gen_startLine(std::map<string, string>::iterator status)
{
	if (_htx["A"].empty())
		_htx["A"] = std::vector<std::string>();
	if (_htx["A"].size() != 3)
		_htx["A"].resize(3, string());
	_htx["A"][0] = "HTTP/1.1 "; // version (static)
	_htx["A"][1] = status->first; // status code (dynamic) -> 200
	_htx["A"][2] = status->second; // status msg (dynamic) -> OK
	_htx["A"][2] += "\r\n";
// MODIFIE LE PNG DS ERROR_PAGE.HTML SI NECESSAIRE
	if (atoi(status->first.c_str()) >= 300) {
		fstream error_file("files/error_pages/4xx.html");
		error_file.seekg(1252);
		error_file.write(status->first.c_str(), 3);
		error_file.close();
	}
}

void	request_handler::gen_date()
{
    std::string	date = "Date: ";
    time_t		timer;
    struct tm	*info;
    char		timestamp[36];

    timer = time(NULL);
    info = localtime(&timer);
    strftime(timestamp, 36, "%a, %d %h %Y %H:%M:%S GMT", info);
    date.append(timestamp);
    date.append("\r\n");

	// _response.append(date);
	if (_htx["Date"].empty())
		_htx["Date"] = std::vector<std::string>();
	_htx["Date"].push_back(date);
}

void	request_handler::gen_serv()
{
	if (_htx["Server"].size() != 3)
		_htx["Server"].resize(3, string());
	_htx["Server"][0] = "Server: ";// HEADER_LABEL
	if (!_hrx["Host:"].empty())
		_htx["Server"][1] = _hrx["Host:"][0];// IP & HOST
	else
		_htx["Server"][1] = "127.0.0.1";// IP & HOST
	_htx["Server"][2] = "\r\n";
}

void	request_handler::gen_CType(/* string ext */) /* PROBLEM : mieux vaudrait extraire ça d'un fichier et le récup ici (serait plus élégant)*/
{
// Capture file.ext(ension)
	// if (ext.empty())
		// ext = _hrx["A"][1].substr(_hrx["A"][1].find_last_of(".") + 1);
	string ext = _path.substr(_path.find_last_of(".") + 1);
#ifdef _debug_
	cout << "file ext asked : " << ext << endl;
#endif
// Content type : https://developer.mozilla.org/fr/docs/Web/HTTP/Basics_of_HTTP/MIME_types
	_htx["Content-Type"].clear();
	if ( ext == "/" || ext == "html" ) // Default file PROBLEM ?
		_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
	else if ( ext == "css" ) // Default file PROBLEM ?
	{
		_htx["Content-Type"].push_back("Content-Type: text/css\r\n");
//		_htx["Connection"].push_back("Connection: close\r\n");
	}
	else if( ext == "ico" || ext == "png" || ext == "jpeg" || ext == "webp" || ext == "gif" || ext == "bmp" )
		_htx["Content-Type"].push_back("Content-Type: image\r\n");
	else if( ext == "ogg" || ext == "wav" || ext == "midi" || ext == "mpeg" || ext == "webm" )
		_htx["Content-Type"].push_back("Content-Type: audio\r\n");
	else if( ext == "ogg" || ext == "mp4" || ext == "webm" )
		_htx["Content-Type"].push_back("Content-Type: video\r\n");
	else // DEFAULT PAREIL QUE L EPREMIER À ARRANGER :) POUR LE TOI DU FUTUR BISOU
		_htx["Content-Type"].push_back("Content-Type: text/html; charset=utf-8\r\n");
		// _htx["Content-Type"].push_back("Content-Type: application/octet-stream\r\n");
}

// génération du header Content-Length (1 = clen, 2 = file_path, 3 = _body.size())
void	request_handler::gen_CLength(int x)
{/* PROBLEME */ // http://127.0.0.1:8080/test les gif 404 s'affiche http://127.0.0.1:8080/test/ <- '/' non
/* PROBLEM */ // http://127.0.0.1:8080/downloads/ n'affiche pas le poulpe -> les autre requetes sont faites sur le folder alors que le path du html est neutre, mais ça ça marche : http://127.0.0.1:8080/downloads 
	cout << BLUE "IN GEN_CLENGTH()" RESET << endl;
	_htx["Content-Length"].clear();

	stringstream ss;
/* 	if (_c->rqst_transfer_t == MULTIPART) {
		ss << _hrx["Content-Length:"][0];
		cout << MAGENTA "1\n" RESET;
	}
	else if (_c->rqst_t != GET && _c->rqst_transfer_t == NO_BODY) {
		cout << MAGENTA "2\n" RESET;
		return ; // PAS  BESOIN DE CONTENT-LEN ON RETURN
	}
	else */if (x == 1) {
		ss << _c->clen;
		cout << MAGENTA "3\n" RESET;
	}
	else if (x == 2) {
		cout << MAGENTA "4\n" RESET;
		ifstream fs(_path.c_str(), std::ifstream::binary | std::ifstream::ate);
		ss << fs.tellg();
		fs.close();
	}
	else {
		cout << MAGENTA "5\n" RESET;
		if (_body.size())
			ss << _body.size();
		else return ;
	}
	_htx["Content-Length"].push_back("Content-Length: "); // HEADER_LABEL -> IL APPARAITTRA ALORS DS LES HEADER _HTX
	_htx["Content-Length"].push_back( ss.str());
	_htx["Content-Length"].push_back( "\r\n"  );
// PLUS DE REQUETE TROP LONGUE POUR LA REPONSE AU CLIENT (A DECOMMENTER -AV SON HOMOLOGUE DS gen_CLength()- SI CHANGEMENT D'AVIS)
/* 	if (!_si[_s_id].location[_l_id].max_file_size.empty() // POUR LES REQUEST ENTITY TOO LARGE SUREMENT A VIRER D'ICI
		&& atoi(ss.str().c_str()) > atoi(_si[_s_id].location[_l_id].max_file_size.c_str()))
		gen_startLine( _status.find("413") ); */
}

	/* FUNCTION SECONDAIRE : UTILITAIRES */

// reconnait quel server_virtuel va traiter la requete et initialise _s_id
void request_handler::set_server_id(void)
{
	size_t colon_pos = _hrx["Host:"][0].find_first_of(":");
	string host(_hrx["Host:"][0].substr(0,colon_pos));
	// PROBLEM : SPARADRAP
//	if (host == "127.0.0.1") host = "localhost";
	string port(_hrx["Host:"][0].substr(colon_pos +1));
	for (size_t i = 0; i < _si.size(); ++i)
		if ( (_si[i].host == host || _si[i].server_name == host) && _si[i].port == port ) {
			_s_id = i;
			cout << RED "_s_id : " RESET << _s_id << endl;
			return ;
		}
// SI LE HOST:PORT N'A PAS ETE TROUVE ON SELECT LE PREMIER SERVER CORRESPONDANT
	for (size_t i = 0; i < _si.size(); ++i)
		if (_si[i].port == port) {
			_s_id = i;
			break;
		}
#ifdef _debug_
			cout << "host : " << host << ", port : " << port << ", id : " << _s_id << endl;
#endif
}

/* ◦ Le premier serveur pour un host :port sera le serveur par défaut pour cet
host :port (ce qui signifie qu’il répondra à toutes les requêtes qui n’appar-
tiennent pas à un autre serveur) */



int request_handler::handle_get_rqst(void)
{
	file_type();
	gen_CType();
	gen_CLength(2);
	add_all_field();
	add_body(false); // add file content
	return 0;
}

       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

int request_handler::handle_put_rqst(void)
{
	cout << BLUE "IN HANDLE_PUT_RQST" RESET << endl;
	// EN CAS DE BODY PLUS LONG QU'AUTORISÉ -> 413 (REQUEST ENTITY TOO LARGE)

	cout << BLUE "1" RESET << endl;

// PROBLEM : NO CHECK NO GOOD (POUR LES VALEURS RETOURNÉES PAR LES FIND)
	if (_c->rqst_transfer_t == MULTIPART) {
		size_t actual_file_size;
		if ((actual_file_size = check_file_size()) == string::npos)
			return 1;
		size_t pos_boundary;
		if ((pos_boundary = _c->rqst.find("\r\n\r\n")) == string::npos) // Vérifie qu'on a au moins les headers
			return 1;
		pos_boundary += 4;
		cout << MAGENTA "_c->rqst.size() / pos_boundary : " RESET << _c->rqst.size() << " / " << pos_boundary <<endl;
		cout << MAGENTA "actual_file_size : " RESET << actual_file_size << endl;
		cout << MAGENTA "_c->rqst : " RESET << _c->rqst << endl;
		if ( (_c->rqst.size() - pos_boundary) < actual_file_size )
			return 1;
		cout << MAGENTA "LA\n" RESET;

		_body = _c->rqst.substr(pos_boundary);
	}
	else if (_c->rqst_transfer_t == URL_ENCODED)
		extract_xform_rqst_body();
	else if (_c->rqst_transfer_t == CHUNCK) {
		cout << BLUE "2" RESET << endl;
		if (extract_chunk_rqst_body())
			cout << RED << "ERROR PUT CHUNK\n" RESET;
	}
	// SI LE FICHIER EXISTE NGINX REPOND : "HTTP/1.1 204 No Content"
	struct stat sb;
	_c->post_file_path = _path + "_PUT";
	if (stat(_c->post_file_path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
		gen_startLine(  _status.find("204") );
	else
		gen_startLine(  _status.find("201") );

	// CREER ET ECRIT DS LE FICHIER, AINSI QUE DS _BODY POUR LA REPONSE
	// int fd;
	// if ((fd = open(_c->post_file_path.c_str(), O_TRUNC | O_CREAT)) != -1/* my_file.is_open() */) {
	// 	write(fd, _body.c_str(), _body.length());
	// 	close(fd);
	ofstream my_file;
	my_file.open(_c->post_file_path, std::ofstream::out | std::ofstream::trunc);
	my_file.seekp(0);
	if (my_file.is_open()) {
		// my_file << _body;
		my_file.write(_body.c_str(), _body.size());
		my_file.close();
		cout << GREEN "FILE WRITTEN !! Upload location : " + _c->post_file_path  + RESET "\n\n" + _c->rqst + "\n\n" ;
	}
	else
		cout << RED "download path invalid : " RESET << _c->post_file_path << "\n";
	// gen_CType();
	// gen_CLength(3);
	add_all_field(); 
	// add_body();
	return 0;
}

int request_handler::handle_post_rqst(void) 
{
	cout << BLUE "IN HANDLE_POST_RQST" RESET << endl;
	// cout << "_hrx['Content-Type:''][1] : " << _hrx["Content-Type:"][0] << endl;
	cout << GREEN "RQST :\n" RESET << "[" + _c->rqst + "]" << endl;
// PROBLEM SI C URL_ENCODE C DS LE FIELD  _hrx["Content-Type:"][0] SINON LA BOUNDARY EST DE LE FIELD _hrx["Content-Type:"][1] D'OU LE SEGV plsu bas si pas de boundary

	// SI TOUT S'EST BIEN PASSÉ ON DÉTACHE LE BODY DE LA REQUETE POST (DS _BODY)
	if (_c->rqst_transfer_t == MULTIPART)
		extract_multi_rqst_body();
	else if (_c->rqst_transfer_t == URL_ENCODED) // Content-Type: application/x-www-form-urlencoded
		extract_xform_rqst_body();
	else if (_c->rqst_transfer_t == CHUNCK)
		extract_chunk_rqst_body();

	struct stat sb;
	_c->post_file_path = _path + "_POST";
	if (stat(_c->post_file_path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
		gen_startLine(  _status.find("204") );
	else
		gen_startLine(  _status.find("201") );

	ofstream my_file(_c->post_file_path, std::ofstream::out | std::ofstream::trunc);
	// my_file.open(_c->post_file_path, std::ofstream::out | std::ofstream::trunc);
	// my_file.seekp(0);
	if (my_file.is_open()) {
		// my_file << _body;
		my_file.write(_body.c_str(), _body.size());
		my_file.close();
		cout << GREEN "FILE WRITTEN !! Upload location : " + _c->post_file_path  + RESET "\n\n" + _c->rqst + "\n\n" ;
	}
	else
		cout << RED "download path invalid : " RESET << _c->post_file_path << "\n";

	add_all_field();
	return 0;
}

// Extrait le body du message de la post request et le mets a la fois ds le _body pour la reponse
// et ds le fichier specifie si ce n'est pas un fichier a traiter par les cgi
int request_handler::extract_multi_rqst_body(void)
{

// PROBLEM SI C URL_ENCODED C DS LE FIELD  _hrx["Content-Type:"][0] SINON LA BOUNDARY EST DS LE FIELD _hrx["Content-Type:"][1] D'OU LE SEGV plsu bas si pas de boundary
	string boundary = _hrx["Content-Type:"][1].substr( _hrx["Content-Type:"][1].find_last_of('-') + 1, string::npos);
	cout << "boundary : " << boundary << endl;
	// cout << endl << "_hrx['Content-Length'][0].c_str() : " << _hrx["Content-Length:"][0].c_str() << endl;

	// EN CAS DE BODY PLUS LONG QU'AUTORISÉ -> 413 (REQUEST ENTITY TOO LARGE)
	if (check_file_size() == string::npos)
		return 1;

	client_info& cl = *_c;
	size_t pos, pos1;
// PROBLEM : NO CHECK NO GOOD (POUR LES VALEURS RETOURNÉES PAR LES FIND)
	size_t pos_boundary;
	if ((pos_boundary = cl.rqst.find("\r\n\r\n")) == string::npos) // Vérifie qu'on a au moins les headers
		return 1;
	pos_boundary += 4;
	size_t pos_last_boundary = cl.rqst.find_last_of("\r\n", cl.rqst.size() - 4);

// GET NAME AND FILENAME INSIDE BOUNDARY
	string name("name=\"");
	string filename("filename=\"");
	// cout << "name [" << name + "]" << endl;
	if ( (pos = cl.rqst.find(name, pos_boundary)) != string::npos )
		if ( (pos1 = cl.rqst.find_first_of('"', pos + name.size())) != string::npos )
			name = cl.rqst.substr(pos + name.size(), pos1 - (pos + name.size()));
	cout << "pos " << pos << " pos1 " << pos1 <<  " name [" << name + "]" << endl;

	// cout << "filename [" << filename + "]" << endl;
	if ( (pos = cl.rqst.find(filename, pos_boundary)) != string::npos )
		if ( (pos1 = cl.rqst.find_first_of('"', pos + filename.size())) != string::npos )
			filename = cl.rqst.substr(pos + filename.size(), pos1 - (pos + filename.size()));
	cout << "pos " << pos << " pos1 " << pos1 <<  " filename [" << filename + "]" << endl;
// END GET NAME AND FILENAME INSIDE BOUNDARY

// CREER ET ECRIT DS LE FICHIER, AINSI QUE DS _BODY POUR LA REPONSE
	cl.post_file_path = _path + '/' + name + "_transfer";
	if ((pos = cl.rqst.find("\r\n\r\n", pos1)) != string::npos && (pos += 4)) {// +=4 == "\r\n\r\n"
		_body = cl.rqst.substr(pos, pos_last_boundary - pos - 1);
		ofstream my_file(cl.post_file_path, std::ofstream::out | std::ofstream::trunc);
		if (my_file.is_open()) {
			my_file << _body;
			my_file.close();
			cout << "Upload location : " + _path + '/' + name + "_transfer\n\n" + cl.rqst + "\n\n";
		}
		else
			cout << RED "download path invalid : " RESET << cl.post_file_path << "\n";
	}
	return 0;
}

int request_handler::extract_xform_rqst_body(void)
{
	cout << BLUE "\nDS extract_xform_rqst_body\n" RESET;
	// cout << "_hrx['Content-Length'][0].c_str() : " << _hrx["Content-Length:"][0].c_str() << endl;

	client_info& cl = *_c;
	size_t pos_boundary;

	// EN CAS DE BODY PLUS LONG QU'AUTORISÉ -> 413 (REQUEST ENTITY TOO LARGE)
	if (check_file_size() == string::npos)
		return 1;

	if ((pos_boundary = cl.rqst.find("\r\n\r\n")) == string::npos) // Vérifie qu'on a au moins les headers
		return 1;
	pos_boundary += 4;
	size_t pos_last_boundary = cl.rqst.find_last_of("\r\n", cl.rqst.size() - 4);
	_body = cl.rqst.substr(pos_boundary, pos_last_boundary - pos_boundary - 1);
#ifdef _debug_
	if (_body.size() <= 1000)
		cout << "[" << _body << "]" << endl;
	else 
		cout << RED "Body too long to be displayed\n" RESET << endl;
#endif
	return 0;
}

int request_handler::extract_chunk_rqst_body(void)
{ cout << BLUE "DS EXTRACT_CHUNK_RQST_BODY\n" RESET;
	_body.clear();
	size_t pos, count = 1;
	if ((pos = _c->rqst.find("\r\n\r\n")) == string::npos) // Vérifie qu'on a au moins les headers
		return 1;
	pos += 2;
	while (count) {
		pos += 2;
		_c->clen += count = strtol(&_c->rqst[pos], NULL, 16);
		cout << "pos : " << pos << ", count : " << count << endl;
		pos = _c->rqst.find_first_of("\n", pos) + 1;
		_body.append(_c->rqst, pos, count);
		pos += count;
	}
	cout << "here is the concatenated string : " << _body.size()  << endl;
#ifdef _debug_
	if (_body.size() <= 1000)
		cout << "[" << _body << "]" << endl;
	else 
		cout << RED "Body too long to be displayed\n" RESET << endl;
#endif
	return 0;
}

// Permet de séléctionner la location qui partage le plus avec l'url,
// identifie l'index de la location correspondante à l'url spcécifiée (_l_id)
// Verifie si la methode est autorisee (maj gen_stratLine 405 ou 403 si besoin)
int	request_handler::resolve_path()
{
	size_t len;
#ifdef _debug_
	cout << GREEN "DS RESOLVE_PATH [" + _hrx["A"][1] + "], _s_id : " << _s_id <<  " _path (cleared afterward): " << _path << endl;
#endif
	_path.clear();
// REMOVE MULTIPLE '/' AND THE '/' AT URL'S END
	clean_url(_hrx["A"][1]);
// MANOUCHERIE A VIRER POUR RECUP LES PAGES D'ERREUR SANS BOUCLE SUR 4XX.HTML SI 405
	if (_hrx["A"][1].find("/error_pages/", 0,13) == 0) {
		_path = "files" + _hrx["A"][1];
		return 1;
	}
// ON ENLEVE LES ARGUMENTS S'IL Y EN A DS L'URL
	if ((len = _path.find("?")) != std::string::npos)
		_path = _path.substr(0, len);

	_l_id = 0;
	int index = _si[_s_id].location.size() - 1;
	len = 0;
	for (vector<locati_info>::reverse_iterator it = _si[_s_id].location.rbegin(); it != _si[_s_id].location.rend(); ++it, --index)
		if ((_hrx["A"][1].find( it->location.c_str(), 0 , it->location.size()) == 0 && it->location.size() > len) || (!len && !index)) // || si on a rien trouvé la location[0] est le default
		{
			// S'IL Y A UNE DIRECTIVE_RETURN À L'INTERIEURE DE LA LOCATION
			if (!it->retour.empty())
				return_directive(it);
			// SINON
			else {
				_path = it->root + "/";
			// SI POST || PUT ET PRESENCE DIRECTIVE_DOWNLOAD À L'INTERIEURE DE LA LOCATION
				if ((_c->rqst_t == POST || _c->rqst_t == PUT) && !it->download_path.empty())
					_path = it->download_path + '/'; // on prend le path_ de download_path tel quel (pas de combinaison av root mettre += si on veut le combiner)
				_l_id = index;
			}
			// _path += it->location.back() == '/' ? it->location : it->location + "/";
			_path += _hrx["A"][1].substr(it->location.size());
			len = it->location.length();
			// if (it->location.size() == _hrx["A"][1].size()) // Mnt ajout de l'index.html mis dorenavant ds file_type si necessaire
			// 	_path += _si[_s_id].location[_l_id].index;
		}
#ifdef _debug_
	cout << BLUE "path : " RESET << _path << endl;
#endif
// AFFACER LES '/' EN PRÉFIXE (POUR OUVRIR DEPUIS LA RACINE DE NOTRE DOSSIER ET PAS DEPUIS LA RACINE MERE)
	while (_path[0] == '/')
		_path.erase(_path.begin());
	clean_url(_path);
#ifdef _debug_
	cout << BLUE "resolved_path : " RESET << _path << endl;
	cout << "location [" << _l_id << "] : " + _si[_s_id].location[_l_id].location << endl;
#endif
// VERIFIE SI LA MÉTHODE DS LA LOCATION CONCERNÉE EST AUTORISÉE (maj gen_stratLine 405 si besoin)
	return !is_method_allowed(); // retourne 1 si allowed d'ou le '!'
}

// Détecte si c'est un fichier normal ou s'il n'existe pas (maj de la statut-line si besoin)
// Si c'est un dossier check l'autoindex ou la presence d'une directive index et affiche le contenu du dossier ou ajoute le fichier index respectivement
// Si erreur lors de l'ouverture du fichier renvoie le _path sur les pages d'erreurs
int request_handler::file_type()
{
	cout << BLUE "DS FILE_TYPE()" RESET <<  " _path : " << _path << endl;
	// struct stat sb = {0}; // à la place de : bzero(&sb, sizeof(sb));
	struct stat sb;
	bzero(&sb, sizeof(sb));
	if (lstat(_path.c_str(), &sb) == -1)
		perror("lstat");

	switch (sb.st_mode & S_IFMT) {
		case S_IFDIR:  printf(RED "directory\n" RESET);
			if (_si[_s_id].location[_l_id].autoindex == "on") {
				generate_folder_list();
				return 1;
			}
// S'IL S'AGIT D'UN DOSSIER DS LEQUEL IL Y A UN INDEX.HTML A RÉCUPÉRER
			if (!_si[_s_id].location[_l_id].index.empty()) {
				_path += _path.back() == '/' ? _si[_s_id].location[_l_id].index : '/' + _si[_s_id].location[_l_id].index;
				file_type();
			}
			break;
		case S_IFREG:  printf(RED "regular file\n" RESET);
			break;
		default:
			if (atoi(_htx["A"][1].c_str()) < 400)
				gen_startLine( _status.find("404") );
			printf(RED "unknown path : %s\n" RESET, _path.c_str());
			break;
	}
// SI ERROR, AIGUILLE LE PATH SUR LA PAGE D'ERREUR CORRESPONDANTE
	if (atoi(_htx["A"][1].c_str()) >= 400)
		_path = _si[_s_id].error_page.empty() || _si[_s_id].error_page.find("files/error_pages") != string::npos ? "files/error_pages/4xx.html" : _si[_s_id].error_page + _htx["A"][1] + ".html";
	printf("_path : %s\n", _path.c_str());
	return 0;
}

// Créé la liste de fichier(s) à afficher ds _body
void request_handler::generate_folder_list()
{
	DIR *dpdf;
	set<string> st;
	struct dirent *epdf;

	dpdf = opendir(_path.c_str());
	if (dpdf != NULL)
	   	while ((epdf = readdir(dpdf))) {
			st.insert(epdf->d_name);
	    	std::cout << epdf->d_name << std::endl;
		}
	closedir(dpdf);

	fstream autoindex_file("configuration_files/autoindex.html");
	if (!autoindex_file.is_open())
		throw (std::runtime_error("Couldn't open : configuration_files/autoindex.html"));
// BUFFERISE LE TEMPLATE HTML POUR Y AJOUTER LE CONTENU DU DOSSIER
	std::stringstream buffer;
	buffer << autoindex_file.rdbuf();
	_body = buffer.str();
	autoindex_file.close();
	// cout << "_hrx['A'][1]" << _hrx["A"][1] << " _path : "  << _path << endl;
	for (set<string>::iterator i = st.begin(); i != st.end(); ++i)
		_body.append("<div style='padding:10px;'><a href=\"" + _hrx["A"][1] + '/' + *i + "\">" + *i + "</a></div>");
	_body.append("</div></body></html>");
}

// Clear la string (response) et y ajoute tous les field
void request_handler::add_all_field()
{
		_c->resp.clear();
	for (std::map<string, vector<string> >::iterator it = _htx.begin(); it != _htx.end(); it++)
		for (size_t i = 0, j = it->second.size(); i < j; ++i)
			_c->resp += it->second[i];
	_c->resp += "\r\n";
	cout << BLUE "DS ADD_ALL_FIELD() all_response_header :\n" RESET << _c->resp << endl;

#ifdef _debug_
	ofstream _LOGfile;
	_LOGfile.open("log.txt", std::ofstream::app);
	if (_LOGfile.is_open()) {
		_LOGfile << "Response :\n";
		string str;
		std::stringstream ss(_c->resp);
		while (getline(ss, str))
			_LOGfile << "> " + str + "\n";
		_LOGfile.close();
	}
#endif
}

// Ajout du fichier ou du body À LA SUITE des header dans response (src == 0 ->use _path, src == value -> use _body)
void request_handler::add_body(int slt_src)
{
	cout << BLUE "DS ADD_BODY()\n" RESET;
	cout << (slt_src ? ("reqst_t :" + _c->rqst_t) : ("_path :" + _path ) ) << endl;
/* 	if (_htx["A"][1] == "413") // PLUS DE REQUETE TROP LONGUE POUR LA REPONSE AU CLIENT
		return ; */
	if (/* _c->rqst_t != PUT_CHUNCK &&  !_body.empty()*/slt_src) {
		cout << RED "Body (folder) written !" RESET  << endl;
		_c->resp += _body;
		_body.clear();
		return ;
	}
// S'IL S'AGIT D'UN GET OU D'UN POST ON JOINS LE FICHIER
	else {
		cout << RED "File written !" RESET  << endl;
		ifstream fs(_path);
		_c->resp.append((istreambuf_iterator<char>(fs)),
						 (istreambuf_iterator<char>() ));
	}
	// else if (_hrx["A"][0] == "POST") {
	// 	cout << RED "Post body written !" RESET  << endl;
	// 	_c->resp.append(_c->resp);
	// }
}

// Verifie si elle est autorisee ds le cas d'une methode inconnue faite av curl -X (maj gen_stratLine 405 si besoin)
// puis si la méthode ds la location concernée est autorisée (maj gen_stratLine 403 si besoin)
bool request_handler::is_method_allowed(void)
{/* PROBLEME (A TESTER) */
	if (_hrx["A"][1] == "/directory/youpi.bla")  // A VIRER SPARADRAP POUR TESTER PROBLEM !!
		return true;  // A VIRER SPARADRAP POUR TESTER PROBLEM !!
	cout << MAGENTA "is_method_allowed : " RESET;
	bool allowed = false;
	const char *array[] = {"GET", "PUT", "HEAD", "POST", "DELETE", "PATCH", NULL};
	for (const char**strs = array; *strs; ++strs){
		// cout << MAGENTA << *strs << RESET << endl;
		if (*strs == _hrx["A"][0])
			allowed = true;
	}
	if (!allowed){
		gen_startLine( _status.find("400") );
		cout << MAGENTA << "400 Bad Request" << RESET << endl;
		return allowed;
	}
	allowed = false;
	for (size_t i = 0; i < _si[_s_id].location[_l_id].allowed_method.size(); ++i)
		if (_si[_s_id].location[_l_id].allowed_method[i] == _hrx["A"][0])
			allowed = true;
	if (!allowed)
		gen_startLine( _status.find("405") );
	cout << MAGENTA << (allowed ? "oui\n" : "non\n") << RESET;
	return allowed;
}

// Pour Resolve_path() : s'il y a une directive_return à l'interieure de la location
void request_handler::return_directive(vector<locati_info>::reverse_iterator& it)
{
	for (vector<locati_info>::iterator it2 = _si[_s_id].location.begin(); it2 != _si[_s_id].location.end(); ++it2)
		if (it2->location == it->retour.back()) {
			cout << RED " it->retour[1] :" RESET +  it->retour[1] << endl;
			string::const_iterator c_it = it->retour[0].begin();
			while (c_it != it->retour[0].end() && std::isdigit(*c_it))  // On verifie qu'il s'agisse slmt de chiffre ...
				++c_it;
			if ( !it->retour[0].empty() && c_it == it->retour[0].end()) // ... d'un status_code ds it->retour[0]
				gen_startLine( _status.find(it->retour[0]) ); // 301 dependament du .conf

			_path = it2->root + "/";
			_l_id = it2 - _si[_s_id].location.begin();
			break ;
		}
}

// Check si le fichier fourni par la rqst est plus long que "max_file_size" defini ds la conf
// maj de la stratLine au cas où : 413 Request Entity Too Large, ds ce cas return npos
size_t request_handler::check_file_size(void)
{
	// EN CAS DE BODY PLUS LONG QU'AUTORISÉ -> 413 (REQUEST ENTITY TOO LARGE)
	if (_hrx.find("Content-Length:") == _hrx.end() || _hrx["Content-Length:"].empty())
		return string::npos;
	size_t max_file_size = atoi(_si[_s_id].max_file_size.c_str());
	size_t actual_file_size = atoi(_hrx["Content-Length:"][0].c_str());
	if (!_si[_s_id].max_file_size.empty() && actual_file_size > max_file_size ) {
		gen_startLine( _status.find("413") ); 
		return string::npos;
	}
	cout << BLUE "DS CHECK_FILE_SIZE, Content-Length: " RESET << actual_file_size << endl;
	return actual_file_size;
}

// Remove multiple '/' and the '/' at url's end
void request_handler::clean_url(string& str)
{
	for (size_t i = 0; i < str.size(); ++i)
		while (str[i] == '/' && (str[i + 1] == '/' || str[i + 1] == '\0') && str.size() > 1)
			str.erase(i, 1);
}


	/* FUNCTION GETTER / SETTER */

std::string &request_handler::get_response(void) {return _response;}


	/* FUNCTION DE DEBUG */

void request_handler::display(void) {
	cout << GREEN ITALIC UNDERLINE "DISPLAY HEADER INFORMATION" RESET GREEN " :" RESET << endl;
	for (map<string, vector<string> >::iterator it = _hrx.begin(), end = _hrx.end(); it != end; ++it) {
		cout << it->first << " ";
		for (size_t i = 0; i < it->second.size(); ++i)
			cout << it->second[i] << ", ";
		cout << endl;
	}
}


// AJOUT POST D'ARTHUR
std::vector<std::string> request_handler::extract_env(std::map<std::string, std::vector<std::string> >& mp,const server_info& _s)
{
	std::vector<std::string>	env;
	std::string			tmp;
	std::string			buf(mp["A"][1]);
	std::string			var;

	tmp =  "REDIRECT_STATUS=CGI";
	env.push_back(tmp);
	tmp = "SERVER_SOFTWARE=";
	for (size_t j = 0; j < _s.server_name.size(); ++j)
		tmp += _s.server_name[j];
	env.push_back(tmp);
	tmp = "SERVER_NAME=";
	tmp += _s.host;
	env.push_back(tmp);
	tmp = "GATEWAY_INTERFACE=CGI/1.1";
	env.push_back(tmp);
	tmp = "SERVER_PROTOCOL=HTTP/1.1";
	env.push_back(tmp);
	tmp = "SERVER_PORT=";
	tmp += _s.port;
	env.push_back(tmp);
	tmp = "REQUEST_METHOD=";
	tmp += mp["A"][0];
	env.push_back(tmp);
	tmp = "PATH_INFO=";
	if (!mp["Path-Info"].empty())
	{
		for (size_t j = 0; j < mp["Path-Info"].size(); ++j)
			tmp+= mp["Path-Info"][j];
	}
	env.push_back(tmp);
	tmp = "PATH_TRANSLATED=";
	if (!mp["Path-Translated"].empty())
	{
		for (size_t j = 0; j < mp["Path-Translated"].size(); ++j)
			tmp+= mp["Path-Translated"][j];
	}
	env.push_back(tmp);
	tmp = "SCRIPT_NAME=";
	if (!mp["Script-Name"].empty())
	{
		for (size_t j = 0; j < mp["Script-Name"].size(); ++j)
			tmp+= mp["Script-Name"][j];
	}
	env.push_back(tmp);
	tmp = "QUERY_STRING=";
	if (!mp["Query-String"].empty())
	{
		for (size_t j = 0; j < mp["Query-String"].size(); ++j)
			tmp+= mp["Query-String"][j];
	}
	env.push_back(tmp);
	tmp = "REMOTE_HOST=";
	if (!mp["Host:"].empty())
	{
		for (size_t j = 0; j < mp["Host:"].size(); ++j)
			tmp+= mp["Host:"][j];
	}
	env.push_back(tmp);
	tmp = "DOCUMENT_ROOT=";
	if (!mp["Document-Root"].empty())
	{
		for (size_t j = 0; j < mp["Document-Root"].size(); ++j)
			tmp+= mp["Document-Root"][j];
	}
	env.push_back(tmp);
	tmp = "CONTENT_TYPE=";
	if (!mp["Content-Type:"].empty())
	{
		for (size_t j = 0; j < mp["Content-Type:"].size(); ++j)
			tmp+= mp["Content-Type:"][j];
	}
	env.push_back(tmp);
	tmp = "CONTENT_LENGTH=";
	if (!mp["Content-Length:"].empty())
	{
		for (size_t j = 0; j < mp["Content-Length:"].size(); ++j)
			tmp+= mp["Content-Length:"][j];
	}
	else
		tmp+="0";
	env.push_back(tmp);
	tmp = "HTTP_ACCEPT=";
	if (!mp["Accept:"].empty())
	{
		for (size_t j = 0; j < mp["Accept:"].size(); ++j)
			tmp+= mp["Accept:"][j];
	}
	env.push_back(tmp);
	tmp = "HTTP_ACCEPT_LANGUAGE=";
	if (!mp["Accept-Language:"].empty())
	{
		for (size_t j = 0; j < mp["Accept-Language:"].size(); ++j)
			tmp+= mp["Accept-Language:"][j];
	}
	env.push_back(tmp);
	tmp = "HTTP_USER_AGENT=";
	if (!mp["User-Agent:"].empty())
	{
		for (size_t j = 0; j < mp["User-Agent:"].size(); ++j)
			tmp+= mp["User-Agent:"][j];
	}
	env.push_back(tmp);
	tmp = "HTTP_REFERER=";
	if (!mp["Referer"].empty())
	{
		for (size_t j = 0; j < mp["Referer"].size(); ++j)
			tmp+= mp["Referer"][j];
	}
	env.push_back(tmp);
	tmp = "TMPDIR=";
	if (!_si[_s_id].location[_l_id].download_path.empty())
		tmp += _si[_s_id].location[_l_id].download_path + '/';
	else if (!_si[_s_id].location[_l_id].root.empty())
		tmp += _si[_s_id].location[_l_id].root;
	env.push_back(tmp);
	return (env);
}

void	request_handler::clean_body()
{
	std::string	tmp;
	std::string	index;
	size_t	pos;
	
	while ((pos = _body.find("\r\n")) != std::string::npos)
	{
		tmp = _body.substr(0, pos + 2);
		_body = _body.substr(pos + 2);
		if (tmp == "\r\n")
			break ;
		if ((pos = tmp.find(":")) != std::string::npos)
		{
			index = tmp.substr(0, pos);
//			cout << "index=" << index << endl;
			if (index != "X-Powered-By")
				_htx[index].push_back(tmp.substr(0, tmp.find("\r\n") + 2));
		}
	}
}

void	request_handler::cgi_var_init()	{
	size_t				len, pos;
	std::string			var;

	//CGI variables initialisation
	if ((len = _path.find("?")) != std::string::npos)
	{
		var = _path.substr(len + 1);
		_path = _path.substr(0, len);
	}
	std::cout << "PATH=" << _path << std::endl;
	_hrx.insert(std::make_pair("Path-Translated", std::vector<std::string>()));
	_hrx["Path-Translated"].push_back(_path);
	_hrx.insert(std::make_pair("Query-String", std::vector<std::string>()));
	_hrx["Query-String"].push_back(var);
	var.clear();
	_hrx.insert(std::make_pair("Path-Info", std::vector<std::string>()));
	pos = _path.find(_si[_s_id].cgi_file_types[ext_id]);
	if (pos != std::string::npos)
	{
		var = _path.substr(pos + 4);
		_path = _path.substr(0, pos + 4);
	}
	if (!var.empty() && (pos = var.find("/")) != std::string::npos)
		var = var.substr(pos + 1);
	_hrx["Path-Info"].push_back(var);
	var.clear();
	_hrx.insert(std::make_pair("Script-Name", std::vector<std::string>()));
	var = (_hrx["A"][1][0] == '/' ? _hrx["A"][1].substr(1) : _hrx["A"][1]);
	_hrx["Script-Name"].push_back(var);
	_hrx.insert(std::make_pair("Document-Root", std::vector<std::string>()));
	pos = _path.find_last_of("/");
	var = _path.substr(0, pos);
	if (var.substr(0, 2) == "./")
		var = var.substr(2);
	_hrx["Document-Root"].push_back(var);
	var.clear();
}

int	request_handler::handle_cgi(void)
{
	cout << BLUE "DS HANDLE_CGI" << endl;
	std::vector<std::string>	env;

	//HERE!
	if (_s_id == -1)
	{
		std::cout << "Invalid server id: " << _s_id << std::endl;
		return (0);
	}
	else
	{
	//EN CHANTIER !!!
		if (_si[_s_id].cgi_path.empty())
		{
			gen_startLine(  _status.find("403") );
			return (0);
		}
		std::cout << "Launching cgi" << std::endl;
		cgi_var_init();
		env = extract_env(_hrx, _si[_s_id]);
		if (go_cgi(/*prend un int *fd[2](*int[2]) */NULL, _si[_s_id].cgi_path, env))// || (redir_fd[0] == -1 || redir_fd[1] == -1))
		{
/* 			if (redir_fd[0] != -1)
			{
				close(redir_fd[0]);
				redir_fd[0] = -1;
			}
			if (redir_fd[1] != -1)
			{
				close(redir_fd[1]);
				redir_fd[1] = -1;
			} */
			return (0);
		}
		_body = _hrx["BODY"][0];
		std::cout << "Buffer copied in body" << std::endl;
		return (1);
	//EN CHANTIER !!!
	}
	return (0);
}

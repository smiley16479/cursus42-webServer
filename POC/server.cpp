#include <sys/select.h>
#include <sys/socket.h> /*socket(), AF_INET, SOCK_STREAM */
#include <arpa/inet.h> /* struct sockaddr/in */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
#define SERVER_PORT 	8080
#define BUFSIZE			4096
#define SOCKETERROR		-1
#define SERVER_BACKLOG	100
typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */

void *handle_connection(int);
int check(int exp, const char *msg);
int accept_new_connection(int server_socket);
int setup_server(short port, int backlog);

int main(int argc, char const *argv[])
{
	int server_socket = setup_server(SERVER_PORT, SERVER_BACKLOG);

	fd_set current_sockets, ready_sockets;
	// initialize my set
	FD_ZERO(&current_sockets);
	FD_SET(server_socket, &current_sockets);

	while (true)
	{
		ready_sockets = current_sockets;
		if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
			perror("select error");
			exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &ready_sockets)) {
				if (i == server_socket) {
					// this is a new connection 
					int client_socket = accept_new_connection(server_socket);
					FD_SET(client_socket, &current_sockets);
				} else {
					// do whatever we do with connection 
					handle_connection(i);
					FD_CLR(i, &current_sockets);
				}
			}
		}


		// do whatever we do with connection.
	}
	
	return 0;
}

int setup_server(short port, int backlog){
	int server_socket;
	SA_IN server_addr;

	check((server_socket = socket(AF_INET, SOCK_STREAM, 0)), //comunication domain ici IPv4, type de stream , protocol utilisé 0 pour celui par default
			"Failed to create socket");

	// initialize the address struct
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	check(bind(server_socket, (SA*)&server_addr, sizeof(server_addr)), "Bind failed...");
	check(listen(server_socket, backlog), "Listen failed...");
	return server_socket;
}

int accept_new_connection(int server_socket) {
	int addr_size = sizeof(SA_IN);
	int client_socket;
	SA_IN client_addr;
	check(client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size), "accept failed");
	
	// print client address
	char client_address[BUFSIZE];
	inet_ntop(AF_INET, &client_addr, client_address, BUFSIZE);
	printf("Client address is : %s\n", client_address);
	
	return client_socket;
}

int check(int exp, const char *msg) {
	if (exp == SOCKETERROR) {
		perror(msg);
		exit(1);
	}
	return exp;
}

void *handle_connection(int client_socket ) {
	char buffer[BUFSIZE];
	size_t bytes_read;
	int msgsize = 0;
	char actualpath[BUFSIZE+1];

	// read the client msg -- the name of the file to read
	while ((bytes_read = read(client_socket, buffer+msgsize, sizeof(buffer)-msgsize-1)))
	{
		msgsize += bytes_read;
		if (msgsize > BUFSIZE-1 || buffer[msgsize-1] == '\n') break;
	}
	check(bytes_read, "recv error");
	buffer[msgsize-1] = 0; // null terminate the msg and remove the \n

	cout << "REQUEST: " << buffer << endl;
	fflush(stdout);

	if(realpath(buffer, actualpath) == NULL){
		cout << "ERROR(bad path): " << buffer << endl;
		cout << "actualPath: " << actualpath << endl;
		close(client_socket);
		return NULL;
	}

	// read file and send its contenst to client
	FILE *fp = fopen(actualpath, "r");
	if (fp == NULL) {
		cout << "ERROR(open): " << buffer << endl;
		close(client_socket);
		return NULL; 
	}

	// read file contents and send them to client
	// note a real program would limitthe client to certain file
	while ((bytes_read = fread(buffer, 1, BUFSIZE, fp)) > 0) {
		// cout << "sending " << bytes_read << endl;
		write(client_socket, buffer, bytes_read);
	}
	close(client_socket);
	fclose(fp);
	cout << "closing connection" << endl;
	return NULL;
}
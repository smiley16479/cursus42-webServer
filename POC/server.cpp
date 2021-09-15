// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>

#define PORT 8080

void log(const std::string& msg)
{
	std::cerr << "\033[1m\033[34m[POC Server log] \033[0m: " << msg << std::endl;
}

void debug_log(const std::string& msg)
{
	std::cerr << "\033[1m\033[30m[POC Server debug] \033[0m: " << msg << std::endl;
}

int main(int argc, char const *argv[])
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char *hello = "Hello from server";
	
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	debug_log("Socket created; server_fd = " + std::to_string(server_fd));
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	debug_log("Socket bind to PORT=" + std::to_string(PORT) + "; server_fd = " + std::to_string(server_fd));

	int backlog = 3;
	if (listen(server_fd, backlog) < 0){ perror("listen"); exit(EXIT_FAILURE); }
	debug_log("Listening server_fd = " + std::to_string(server_fd) + " with backlog of " + std::to_string(backlog));

	while (1)
	{

		log("Waiting for connection...");
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

		debug_log("New connection new_socket = " + std::to_string(new_socket));

		valread = read( new_socket , buffer, 1024);
		log("Client message (socket = " + std::to_string(new_socket) + ") = " + buffer );
		send(new_socket , hello , strlen(hello) , 0);
		log("Closing connection (socket = " + std::to_string(new_socket) + ")");
		close(new_socket);
	}
	return 0;
}

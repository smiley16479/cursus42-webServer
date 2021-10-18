# WebServer

## Resources

- [History of HTTP versions](http://www.tcpipguide.com/free/t_HTTPOverviewHistoryVersionsandStandards.htm)
- [More Resources](https://github.com/artainmo/webserv)
- [htons little-endian big-endian](https://stackoverflow.com/a/19209503/12416249)
  - [htonl, htons, ntohl, ntohs](http://www.man-linux-magique.net/man3/htonl.html)

### Jacob Sorber

Some sample code in the `POC` folder.

1. [Simple Web Browser](https://www.youtube.com/watch?v=bdIiTxtMaKA)
2. [Simple Web Server](https://www.youtube.com/watch?v=esXw4bdaZkc)
3. [Read the client data](https://www.youtube.com/watch?v=1jv428xKsRg)
4. [Select (does not build from the last videos so the code is not the same)](https://www.youtube.com/watch?v=Y6pFtgRdUts)


## Résumé fonctions & structures

 NOTES
   To accept connections, the following steps are performed:

        1.  A socket is created with socket(2).

        2.  The socket is bound to a local address using bind(2), so that other sockets may be connect(2)ed to it.

        3.  A willingness to accept incoming connections and a queue limit for incoming connections are specified with listen().

        4.  Connections are accepted with accept(2).

See getaddrinfo(3) pour un exemple server/client complet :
-	Pas besoin de connect(2) dans un server, parcontre neccessaire dans un client.
	int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen); 
	The  connect() system call connects the socket referred to by the file descriptor sockfd to the
       address specified by addr.  The addrlen argument specifies the size of addr.  The format of the
       address in addr is determined by the address space of the socket sockfd; see socket(2) for fur‐
       ther details.

.1 int socket(int domain, int type, int protocol);

	socket()  creates  an  endpoint  for communication and returns a file descriptor that refers to that 	endpoint. (...)
	The  domain  argument  specifies a communication domain; this selects the protocol family which will be used for communication. see man for domain

.2 int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

   struct sockaddr {
       sa_family_t sa_family;
       char        sa_data[14];
   }
   The  only purpose of this structure is to cast the structure pointer passed in addr in order to
   avoid compiler warnings.  See more EXAMPLE in bind(2).

.3 int listen(int sockfd, int backlog);

	listen()  marks the socket referred to by sockfd as a passive socket, that is, as a socket that
    will be used to accept incoming connection requests using accept(2). (...) backlog is the maximum 
	number of pending connection (== request ?)


.4 int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

	The argument sockfd is a socket that has been created with socket(2),
	bound to a local address with bind(2), and is listening for connections after a listen(2).
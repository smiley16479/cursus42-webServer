#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define SERVER_PORT 18000
#define MAX_LINE 4096
#define SA struct sockaddr

void err_n_die(char * str)
{
    printf("error: %s\n", str);
    exit(1);
}

int main(int ac, char **av)
{
    int listenfd, connfd, n;
    struct sockaddr_in servaddr;
    uint8_t buff[MAX_LINE+1];
    uint8_t receiveline[MAX_LINE+1];

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_n_die("socket error");
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);

    if ((bind(listenfd, (SA *) &servaddr, sizeof(servaddr))) < 0)
        err_n_die("bind error");
    
    if ((listen(listenfd, 10)) < 0)
        err_n_die("listen error");

    // fd_set current_socket, ready_socket;
    // FD_ZERO(&current_socket);
    // FD_SET(listenfd, &current_socket);

    for ( ; ; )
    {
        struct sockaddr_in addr;
        socklen_t addr_len;
        char client_address[MAX_LINE+1];

        printf("Waiting for connection on port: %d\n", SERVER_PORT);
        fflush(stdout);
        connfd = accept(listenfd, (SA *) &addr, &addr_len);

        // print client info
        inet_ntop(AF_INET, &addr, client_address, MAX_LINE);
        printf("Client connection: %s\n", client_address);

        memset(receiveline, 0, MAX_LINE);

        while ( (n = read(connfd, receiveline, MAX_LINE - 1)) > 0)
        {
            fprintf(stdout, "\n%s", receiveline);

            if (receiveline[n-1] == '\n')
                break ;
            memset(receiveline, 0, MAX_LINE);
        }
        if (n < 0)
            err_n_die("error reading");
        
        snprintf((char*)buff, sizeof(buff), "HTTP/1.0 200 OK\r\n\r\nHello");
        write(connfd, (char*)buff, strlen((char *)buff));
        close(connfd);
        while (1);
    }

}
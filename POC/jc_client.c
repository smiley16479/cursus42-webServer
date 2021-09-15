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
    int n;
    int sockfd;
    int sendbytes;
    struct sockaddr_in servaddr;
    char    sendline[MAX_LINE];
    char    recieveline[MAX_LINE];

    if (ac != 2)
        err_n_die("give only ip address as argument");
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        err_n_die("socket could not be created");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT); // "host to network short"

    // convert ip addr from string to binary
    if (inet_pton(AF_INET, av[1], &servaddr.sin_addr) <= 0)
    {
        printf("inet_pton error for %s", av[1]);
        exit(1);
    }

    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
        err_n_die("connection failed");
    
    sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
    sendbytes = sizeof(sendline);

    if (write(sockfd, sendline, sendbytes) != sendbytes)
        err_n_die("write error");

    memset(recieveline, 0, MAX_LINE);

    // Read server response
    while ( (n = read(sockfd, recieveline, MAX_LINE - 1)) > 0)
    {
        printf("%s", recieveline);
        memset(recieveline, 0, MAX_LINE);
    }
    if (n < 0)
        err_n_die("read error");
    exit(0);
}

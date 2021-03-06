#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

# define RESET			"\033[0m"
# define RED			"\033[31m"				/* Red */
# define GREEN			"\033[32m"				/* Green */
# define YELLOW			"\033[33m"				/* Yellow */
# define BLUE_BOLD		"\033[1m\033[34m"		/* Yellow */


typedef struct		s_client 
{
	int				fd;
    int             id;
	struct s_client	*next;
}	t_client;

t_client *g_clients = NULL;

int sock_fd, g_id = 0;
fd_set curr_sock, cpy_read, cpy_write;
char msg[42];
char str[42*4096], tmp[42*4096], buf[42*4096 + 42];

void ft_log(char *msg, int msg2)
{
    if (msg2 == 404)
        printf(BLUE_BOLD "[MiniServ] :" RESET " %s\n" , msg);
    else 
        printf(BLUE_BOLD "[MiniServ] :" RESET " %s %d\n" , msg, msg2);
}

void	fatal() 
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	close(sock_fd);
	exit(1);
}

int get_id(int fd)
{
    t_client *temp = g_clients;

    while (temp)
    {
        if (temp->fd == fd)
            return (temp->id);
        temp = temp->next;
    }
    return (-1);
}

int		get_max_fd() // iter ds la liste chainée pour trouver le + gd fd
{
	int	max = sock_fd;
    t_client *temp = g_clients;

    while (temp)
    {
        if (temp->fd > max)
            max = temp->fd;
        temp = temp->next;
    }
    return (max);
}

void	send_all(int fd, char *str_req) // renvoi le msg à tous les clients sauf a l'emetteur
{
    t_client *temp = g_clients;

    while (temp)
    {
        printf("(send_all) temp->fd : %d, fd : %d\n", temp->fd, fd);
        if (temp->fd != fd && FD_ISSET(temp->fd, &cpy_write))
        {
            printf("(send_all) temp->fd != fd\n");
            if (send(temp->fd, str_req, strlen(str_req), 0) < 0)
                fatal();
        }
        temp = temp->next;
    }
}

int		add_client_to_list(int fd)
{// Créer un nouveau maillon avec un id (n+1) et renvoi cet id pour le msg ds add_client
    t_client *temp = g_clients;
    t_client *new;

    if (!(new = calloc(1, sizeof(t_client))))
        fatal();
    new->id = g_id++;
    new->fd = fd;
    new->next = NULL;

    if (!g_clients)
    {
        g_clients = new;
    }
    else
    {
        while (temp->next)
            temp = temp->next;
        temp->next = new;
    }
    printf(GREEN "[MiniServ] : client with fd=%d added as the #%d client\n" RESET, fd, new->id);
    return (new->id);
}

void add_client()
{
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int client_fd;

    ft_log("add_client", 404);
    if ((client_fd = accept(sock_fd, (struct sockaddr *)&clientaddr, &len)) < 0)
        fatal();
    sprintf(msg, "server: client %d just arrived\n", add_client_to_list(client_fd));
    send_all(client_fd, msg);
    FD_SET(client_fd, &curr_sock);
}

int rm_client(int fd)
{
    t_client *temp = g_clients;
    t_client *del;
    int id = get_id(fd);

    if (temp && temp->fd == fd)
    {
        g_clients = temp->next;
        free(temp);
    }
    else
    {
        while(temp && temp->next && temp->next->fd != fd)
            temp = temp->next;
        del = temp->next;
        temp->next = temp->next->next;
        free(del);
    }
    return (id);
}

void ex_msg(int fd)
{
    int i = 0;
    int j = 0;

    while (str[i])
    {
        tmp[j] = str[i];
        j++;
        if (str[i] == '\n')
        {
            sprintf(buf, "client %d: %s", get_id(fd), tmp);
            send_all(fd, buf);
            j = 0;
            bzero(&tmp, strlen(tmp));
            bzero(&buf, strlen(buf));
        }
        i++;
    }
    bzero(&str, strlen(str));
}

/* int main(int ac, char **av)
{
    if (ac != 2)
    {
        write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
        exit(1);
    }
    struct sockaddr_in servaddr;
    uint16_t port = atoi(av[1]);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(port);

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        fatal();
    if (bind(sock_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        fatal();
    if (listen(sock_fd, 0) < 0)
        fatal();
    
    FD_ZERO(&curr_sock);
    FD_SET(sock_fd, &curr_sock);
    bzero(&tmp, sizeof(tmp));
    bzero(&buf, sizeof(buf));
    bzero(&str, sizeof(str));
    ft_log("Connected to 127.0.0.1:", atoi(av[1]));
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    ft_log("sock_fd = ", sock_fd);
    while(1)
    {
        cpy_write = cpy_read = curr_sock;
        if (select(get_max_fd() + 1, &cpy_read, &cpy_write, NULL, NULL) < 0)
            continue; // Si error select -> return -1
        ft_log("iterate over all the fds ...", 404);
        for (int fd = 0; fd <= get_max_fd(); fd++)
        {
            if (FD_ISSET(fd, &cpy_read))
            {// Qd un client donne qlq chose à lire (il ecrit qlq chose)
                ft_log(RED "FD_ISSET true" RESET, fd);
                // return 10;
                if (fd == sock_fd) // Si le fd correspond à celui du server c'est qu'il y a nouvelle connection
                {
                    ft_log("fd == sock_fd", 404);
                    bzero(&msg, sizeof(msg));
                    add_client();
                    break;
                }
                else
                {
                    // close(fd);
                    int n = 0;
                    printf(YELLOW "avt recv\n" RESET);
                    if ((n = recv(fd, str, sizeof(str), 0)) <= 0 && printf(YELLOW "apres recv\n" RESET)) // SI 0 -> EOF, SI -1 error signifie que le client est soit partie soit renontre une erreur
                    {// recv est équivalent à read sauf qu'il a des flags
                        bzero(&msg, sizeof(msg));
                        sprintf(msg, RED "server: client %d just left\n" RESET, rm_client(fd));
                        send_all(fd, msg);
                        FD_CLR(fd, &curr_sock);
                        close(fd);
                        printf(YELLOW "Recv return %d\n" RESET, n);
                        break;
                    }
                    else // on envoie le msg a tous le monde
                    {
                        printf(YELLOW "apres recv ds le else\n" RESET);
                        ex_msg(fd);
                    }
                    while ((n = read(fd, msg, 4)))
                        printf("msg : %s, n = %d\n" , msg,  n);
                    printf(YELLOW "read : end\n" RESET);
                    printf(YELLOW "Fin else\n" RESET);
                    sleep(2);
                }
            }
            
        }
        
    }
    return (0);
} */

 #include <fcntl.h>

int main(int ac, char **av)
{
    if (ac != 2)
    {
        write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
        exit(1);
    }
    struct sockaddr_in servaddr;
    uint16_t port = atoi(av[1]);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(port);

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        fatal();
    if (bind(sock_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        fatal();
    if (listen(sock_fd, 0) < 0)
        fatal();
    
    FD_ZERO(&curr_sock);
    FD_SET(sock_fd, &curr_sock);
    bzero(&tmp, sizeof(tmp));
    bzero(&buf, sizeof(buf));
    bzero(&str, sizeof(str));
    ft_log("Connected to 127.0.0.1:", atoi(av[1]));

    ft_log("sock_fd = ", sock_fd);
    printf("sizeof(cpy_read) : %lu\n", sizeof(cpy_read));
    while(1)
    {

        cpy_write = cpy_read = curr_sock;
        if (select(get_max_fd() + 1, &cpy_read, NULL /* &cpy_write */, NULL, NULL/* &tv */) < 0){
            printf("select error\n");
            continue; // Si error select -> return -1    
        }
        ft_log("iterate over all the fds ...", 404);
        // FD_ZERO(&curr_sock);

{// se bloque av ca
        for (int fd = 0; fd <= get_max_fd(); fd++)
        {
            if (FD_ISSET(fd, &cpy_read))
            {
                printf("avt accept\n");
                if (sock_fd == fd) // nouveau client
                    add_client();
                else {
                    int n = 0;
                    printf("avt recv\n");
                    if ((n = recv(fd, str, sizeof(str), 0)) <= 0 && printf(YELLOW "apres recv\n" RESET)) {
                        FD_CLR(fd, &curr_sock);
                        close(fd);
                        printf("Client closed\n");
                    }
                    else if (printf("apres recv\n") && send(fd, str, strlen(str), 0) < 0)
                        fatal();
                }
            }
        }
        sleep(2);
}

// add_client();

/*         for (int fd = 0; fd <= get_max_fd(); fd++)
        {
            if (FD_ISSET(fd, &cpy_read))
            {// Qd un client donne qlq chose à lire (il ecrit qlq chose)
                ft_log(RED "FD_ISSET true" RESET, fd);
                // return 10;
                if (fd == sock_fd) // Si le fd correspond à celui du server c'est qu'il y a nouvelle connection
                {
                    ft_log("fd == sock_fd", 404);
                    bzero(&msg, sizeof(msg));
                    add_client();
                    break;
                }
                else
                {
                    // close(fd);
                    int n = 0;
                    printf(YELLOW "avt recv\n" RESET);
                    if ((n = recv(fd, str, sizeof(str), 0)) <= 0 && printf(YELLOW "apres recv\n" RESET)) // SI 0 -> EOF, SI -1 error signifie que le client est soit partie soit renontre une erreur
                    {// recv est équivalent à read sauf qu'il a des flags
                        bzero(&msg, sizeof(msg));
                        sprintf(msg, RED "server: client %d just left\n" RESET, rm_client(fd));
                        send_all(fd, msg);
                        FD_CLR(fd, &curr_sock);
                        close(fd);
                        printf(YELLOW "Recv return %d\n" RESET, n);
                        break;
                    }
                    else // on envoie le msg a tous le monde
                    {
                        printf(YELLOW "apres recv ds le else %d\n" RESET, n);
                        ex_msg(fd);
                    }
                    // while ((n = read(fd, msg, 4)))
                    //     printf("msg : %s, n = %d\n" , msg,  n);
                    // printf(YELLOW "read : end\n" RESET);
                    printf(YELLOW "Fin else\n" RESET);
                }
            }
            
        }
        sleep(2); */
    }
    return (0);
}
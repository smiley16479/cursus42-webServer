#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char const *argv[])
{
    char str1[] = "abcdefghijklmnopqrstuvwxyz";
    char str2[50];

    int fd = open("./to_read.txt", O_RDONLY);
    int n = 0;
    while (( n = read(fd, str1, 5)) && n)
        printf("str2 :\"%s\", n : %d\n", str1, n);
    printf("\033[31m""str2 :\"%s\", n : %d\n""\033[0m", str1, n);

    return 0;
}



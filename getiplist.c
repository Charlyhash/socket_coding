// get local ip
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>


#define ERR_EXIT(m)\
    do{\
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)

int main()
{
    char host[1024] = {0};
    if (gethostname(host, sizeof(host)) < 0)
        ERR_EXIT("gethostname error");

    struct hostent* hp;
    if ((hp = gethostbyname(host)) == NULL)
        ERR_EXIT("gethostbyname error");

    int i = 0;
    while (hp->h_addr_list[i] != NULL)
    {
        printf("%s\n", inet_ntoa(*(struct in_addr*)hp->h_addr_list[i]));
        i++;
    }

//    char ip[16] = {0};

    return 0;
}

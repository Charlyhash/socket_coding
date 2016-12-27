//simple  p2p client program
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define ERR_EXIT(m)\
    do{\
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)

int main()
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
        ERR_EXIT("socket error!");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family =AF_INET;
    servaddr.sin_port = htons(7777);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("connet error");

    pid_t pid;
    pid = fork();
    if (pid == -1)
        ERR_EXIT("fork error!");
    if (pid == 0){
        char recvbuf[1024];
        while (1){
            memset(recvbuf, 0, sizeof(recvbuf));
            int ret = read(sock, recvbuf, sizeof(recvbuf));
            if (ret == -1)
                ERR_EXIT("read error");
            else if (ret == 0){
                printf("peer close");
                break;//显式关闭
            }
            fputs(recvbuf, stdout);
        }
        close(sock);
    }
    else {

        char sendbuf[1024] = {0};
        while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL){
            write(sock, sendbuf, strlen(sendbuf));
            memset(sendbuf, 0, sizeof(sendbuf));
         }
    }
    close(sock);

    return 0;
}

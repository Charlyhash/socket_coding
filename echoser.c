//simple server program
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define ERR_EXIT(m)\
    do{\
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)

int main(){
    int listenfd;
    if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket error!");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(7777);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt error");
    if (bind(listenfd, (struct sockaddr*)(&servaddr), sizeof(servaddr)) < 0)
        ERR_EXIT("BIND ERROR");
    if (listen(listenfd, SOMAXCONN) < 0)
        ERR_EXIT("listen error");

    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    int conn;
    if ((conn = accept(listenfd, (struct sockaddr*)(&peeraddr), &peerlen))<0)
        ERR_EXIT("accept error");
    printf("recv connetc ip = %s , port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

    char recvbuf[1024];

    while (1){
        memset(recvbuf, 0, sizeof(recvbuf));
        int ret = read(conn, recvbuf, sizeof(recvbuf));
        if (ret == -1)
            ERR_EXIT("read error!");
        else if (ret == 0){
            printf("peer closed!");
            break;
        }
        fputs(recvbuf, stdout);
        write(conn, recvbuf, ret);
    }

    close(conn);
    close(listenfd);

    return 0;
}

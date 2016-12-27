//simple client program
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

struct packet{
    int len;
    char buf[1024];
};

ssize_t readn(int fd, void* buf, size_t count){
    size_t nleft = count;
    size_t nread;
    char* bufp = (char*)buf;
    while (nleft > 0){
            if ((nread = read(fd, bufp,nleft)) < 0){
                        if (errno == EINTR)
                            continue;
                        return -1;
                    }
            else if (nread == 0){
                        printf("peer close\n");
                        return count-nleft;
                    }

            bufp += nread;
            nleft -= nread;
        }

    return count;
}

ssize_t writen(int fd, void* buf, size_t count){
    int nleft = count; size_t nwrite;
    char* bufp = (char*)buf;
    while (nleft > 0){
            if ((nwrite = write(fd, bufp, nleft)) <0){
                        if (errno == EINTR){
                                        continue;
                                    }
                        return -1;
                    }
            else if (nwrite == 0)
                continue;
            nleft -= nwrite;
            bufp += nwrite;
        }

    return count;
}

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

    struct packet sendbuf;
    struct packet recvbuf;
    memset(&sendbuf, 0, sizeof(sendbuf));
    memset(&recvbuf, 0, sizeof(recvbuf));
    int n;
    while (fgets(sendbuf.buf, sizeof(sendbuf.buf), stdin) != NULL){
        n = strlen(sendbuf.buf);
        sendbuf.len = htonl(n);//数据的长度
        writen(sock, &sendbuf, 4+n);//发送：长度+数据
        int ret = readn(sock, &recvbuf.len, 4);
        if (ret == -1)
            ERR_EXIT("read error");
        else if (ret < 4){
            printf("peer closed!");
            break;
        }
        n = ntohl(recvbuf.len);
        ret = readn(sock, recvbuf.buf, n);
        if (ret == -1)
            ERR_EXIT("read error");
        else if (ret < n){
            printf("peer closed");
            break;
        }
        fputs(recvbuf.buf, stdout);
        memset(&sendbuf, 0, sizeof(sendbuf));
        memset(&recvbuf, 0, sizeof(recvbuf));
    }

    close(sock);

    return 0;
}

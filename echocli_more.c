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

//recv 只能读取套接字，而不能是一般的文件描述符
ssize_t recv_peek(int sockfd, void* buf, size_t len)
{
    while(1){
        int ret = recv(sockfd, buf, len, MSG_PEEK);// 读取后不清空缓冲区
        if(ret == -1 && errno == EINTR) //读取中断
            continue;
        return ret;
    }
}

//读取到'\n'就返回

ssize_t readn(int fd, void* buf, size_t count){
    size_t nleft = count;
    size_t nread;
    char* bufp = (char*)buf;
    while (nleft > 0){
            if ((nread = read(fd, bufp,nleft)) < 0)
            {
                if (errno == EINTR)
                    continue;
                return -1;
            }
            else if (nread == 0)
            {
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
    while (nleft > 0)
    {
        if ((nwrite = write(fd, bufp, nleft)) <0)
        {
            if (errno == EINTR)
            {
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

ssize_t readline(int sockfd, void* buf, size_t maxline)
{
    int ret;
    int nread;
    char* bufp = (char*)buf;
    int nleft = maxline;
    int count = 0;

    while (1){
        ret = recv_peek(sockfd, bufp, nleft);//没有将缓冲区的数据移除
        if (ret < 0){
            return ret; //返回<0表示失败
        }
        else if (ret == 0)
            return ret; //返回0表示对方关闭

        nread = ret;
        int i;
        for (i = 0; i < nread; ++i)
        {
            if(bufp[i] == '\n')
            {
                ret = readn(sockfd, bufp, i+1);//加上空格，使用readn移除
                if(ret != i+1)
                    exit(EXIT_FAILURE);

                return ret +count;
            }
        }
        if (nread > nleft)
            exit(EXIT_FAILURE);
        nleft -= nread; //剩余字节
        ret = readn(sockfd, bufp, nread);
        if (ret != nread)
            exit(EXIT_FAILURE);

        bufp += nread;
        count += nread;
    }

    return -1;
}

void do_echocli(int sock)
{

    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};

    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        writen(sock, sendbuf, strlen(sendbuf));//
        int ret = readline(sock, recvbuf, sizeof(recvbuf));
        if (ret == -1)
            ERR_EXIT("read error");
        else if (ret ==  0){
            printf("peer closed!");
            break;
        }
        fputs(recvbuf, stdout);
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    close(sock);
}


int main()
{
    int sock[5];
    for (int i = 0; i < 5; ++i)
    {
        if ((sock[i] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
            ERR_EXIT("socket error!");

        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family =AF_INET;
        servaddr.sin_port = htons(7777);
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sock[i], (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
            ERR_EXIT("connet error");

        //获取本机地址
        struct sockaddr_in localaddr;
        socklen_t addrlen = sizeof(localaddr);
        if (getsockname(sock[i], (struct sockaddr*)&localaddr, &addrlen) < 0 )
            ERR_EXIT("getsockname error");
        printf("ip = %s, port = %d\n", inet_ntoa(localaddr.sin_addr),
            ntohs(localaddr.sin_port));
    }
    do_echocli(sock[0]);

    return 0;
}

//simple server program using fork
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
#include <signal.h>
#include <sys/wait.h>

#define ERR_EXIT(m)\
    do{\
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)


ssize_t readn(int fd, void* buf, size_t count){
    size_t nleft = count;
    size_t nread;
    char* bufp = (char*)buf;
    while (nleft > 0)
    {
        if ((nread = read(fd, bufp,nleft)) < 0)
        {
            if (errno == EINTR)
                 continue;
             return -1;
        }
        else if (nread == 0)
        {
            printf("peer close\n");
            return count-nleft;
        }
        bufp += nread;
        nleft -= nread;
    }

    return count;
}

ssize_t writen(int fd, void* buf, size_t count)
{
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

//recv 只能读取套接字，而不能是一般的文件描述符
ssize_t recv_peek(int sockfd, void* buf, size_t len)
{
    while(1)
    {
        int ret = recv(sockfd, buf, len, MSG_PEEK);// 读取后不清空缓冲区
        if(ret == -1 && errno == EINTR) //读取中断
            continue;
        return ret;
    }
}

ssize_t readline(int sockfd, void* buf, size_t maxline)
{
    int ret;
    int nread;
    char* bufp = (char*)buf;
    int nleft = maxline;
    int count = 0;

    while (1)
    {
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

void do_service(int conn){
    char recvbuf[1024]; //接收数据存放
    while (1)
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        int ret = readline(conn, recvbuf, 1024); //调用readline函数一次读取一行
                                                // 读取完成后返回
        if (ret == -1){// == -1 error
            ERR_EXIT("read error");
        }
        if(ret == 0){// == 0 closed
            printf("peer closed\n");
            break;
        }
        fputs(recvbuf, stdout); //正常输出
        writen(conn, recvbuf, strlen(recvbuf));
    }
}

void handle_sigchld(int sig)
{

    //wait(NULL);
    while(waitpid(-1, NULL, WNOHANG) < 0)
        ;
}

int main()
{
   // signal(SIGCHLD, SIG_IGN);
   signal(SIGCHLD, handle_sigchld);
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

    pid_t pid;
    while (1)
    {
        if ((conn = accept(listenfd, (struct sockaddr*)(&peeraddr), &peerlen))<0)
            ERR_EXIT("accept error");
        printf("recv connetc ip = %s , port = %d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
        pid = fork();
        if (pid == -1)
        {
            ERR_EXIT("fork error!");
        }
        if (pid == 0){
        //child process
            close(listenfd);
            do_service(conn);
            exit(EXIT_SUCCESS);
        }
        else
            close(conn);//parent process
    }

    return 0;
}

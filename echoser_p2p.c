//simple server p2p
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

#define ERR_EXIT(m)\
    do{\
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)

//父进程退出后通知子进程的信号，否则子进程没有退出
void handle(int sig){
    printf("recv a sig = %d\n", sig);
    exit(EXIT_SUCCESS);
}

void do_service(int conn){
    char recvbuf[1024];
    while (1){
        memset(recvbuf, 0, sizeof(recvbuf));
        int ret = read(conn, recvbuf, sizeof(recvbuf));
        if (ret == 0){
            printf("client close\n");
            break;
        }
        else if(ret == -1){
            ERR_EXIT("READ ERROR!");
        }

        fputs(recvbuf, stdout);
        write(conn, recvbuf, ret);
    }
}
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
    if((conn = accept(listenfd, (struct sockaddr*)(&peeraddr), &peerlen)) < 0)
            ERR_EXIT("saccept error!");
    printf("recv connect ip = %s port = %d\n", inet_ntoa(peeraddr.sin_addr),\
            ntohs(peeraddr.sin_port));

    pid_t pid;
    pid = fork();//创建一个进程：发送数据
    if (pid == -1){
        ERR_EXIT("FORK ERROR!");
    }
    if (pid == 0){//子进程：发送数据
        signal(SIGUSR1, handle);

        char sendbuf[1024]= {0};
        //接收一行数据
        while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL){
            write(conn, sendbuf, strlen(sendbuf));//发送出去
            memset(sendbuf, 0, sizeof(sendbuf));//发送完清空，一遍下一次用
        }
        //发送成功
        exit(EXIT_SUCCESS);
    }
    else{//父进程：获取数据
        char recvbuf[1024];
        while(1){
            memset(recvbuf, 0, sizeof(recvbuf));
            int ret = read(conn, recvbuf, sizeof(recvbuf));
            if (ret == -1){
                ERR_EXIT("read error");
            }
            else if (ret == 0){//对方关闭了
                printf("peer close\n");
                break;
            }
            //输出
            fputs(recvbuf, stdout);
        }
        kill(pid, SIGUSR1);//父进程退出时发送信号给子进程
        exit(EXIT_SUCCESS);
    }

    return 0;
}

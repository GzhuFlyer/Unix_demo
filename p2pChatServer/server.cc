#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define ERR_EXIT(M) \
        do  \
        {   \
            perror(M);  \
            exit(EXIT_FAILURE); \
        }while(0)

void handler(int sig)
{
    printf("recv a sig=%d\n",sig);
    exit(EXIT_SUCCESS);
}
int main(void)
{
    int listenfd;
    if((listenfd = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = PF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //服务器重新启动处于 TIME_WAIT 状态
    //使用REUSEADDR选项可以使得不必等待TIME_WAIT状态消失就可以重启服务器
    int on = 1;
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0)
        ERR_EXIT("setsockopt");
    if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
        ERR_EXIT("bind");
    if(listen(listenfd,SOMAXCONN) < 0)
        ERR_EXIT("listen");
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    int conn;
    pid_t pid;
    if((conn = accept(listenfd,(struct sockaddr*)&peeraddr,&peerlen)) < 0)
        ERR_EXIT("accept");
    //将ip地址转换成点分十进制的ip地址
    printf("ip=%s,port=%d\n",inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));
    //当接收到一个连接，fork一个进程
    pid = fork();   //多进程
    if(pid == -1)
        ERR_EXIT("fork");
    if(pid == 0)
    {
        signal(SIGUSR1,handler);    //接收到父进程退出的信号在handler做处理
        char sendbuf[1024] = {0};
        while (fgets(sendbuf,sizeof(sendbuf),stdin) != NULL)    
        {
            write(conn,sendbuf,strlen(sendbuf));
            memset(sendbuf,0,strlen(sendbuf));
        }
        printf("child close\n");
        exit(EXIT_SUCCESS);
    }else
    {
        char recvbuf[1024];
        while(1)
        {
            memset(recvbuf,0,sizeof(recvbuf));
            int ret = read(conn,recvbuf,sizeof(recvbuf));
            if(ret == -1)
                ERR_EXIT("read");
            else if(ret == 0)
            {
                printf("peer close\n");
                break;
            }
            fputs(recvbuf,stdout);
        }
        printf("parent close\n");
        kill(pid,SIGUSR1);  //父进程退出时通过信号通知子进程
        exit(EXIT_SUCCESS);
    }
    
    // do_service(conn);
    // exit(EXIT_SUCCESS);//当客户端返回时，为客户端开辟的子进程应该退出，退出子进程
    return 0;
}       
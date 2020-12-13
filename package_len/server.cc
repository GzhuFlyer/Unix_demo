#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define ERR_EXIT(M) \
        do  \
        {   \
            perror(M);  \
            exit(EXIT_FAILURE); \
        }while(0)
struct packet
{
    int len;
    char buf[1024];
};
ssize_t readn(int fd, void *buf, size_t count) 
{
    size_t nleft = count;
    ssize_t nread = 0;  //接收到的字节数
    char *bufp = (char*)buf;
    while(nleft > 0)
    {
        nread = read(fd,bufp,nleft);
        if(nread < 0)
        {
            if(errno == EINTR)  //被信号中断,全局变量为errno??
                    continue;   //不认为出错
            return -1;
        }else if(nread == 0)
            return count - nleft;  //对等方关闭，返回已经读取到的字节数
        bufp += nread;  //指针指向的数据进行偏移
        nleft -= nread; //剩余读取的字节＝应该读取的字节数-读取到的字节数
        // break;
    }
    return count;
}   
ssize_t writen(int fd, const void *buf, size_t count)
{
    size_t nleft = count;
    ssize_t nwritten;
    char *bufp = (char*)buf;
    while(nleft > 0)
    {
        nwritten = write(fd,bufp,nleft);
        if(nwritten < 0)
        {
            if(errno == EINTR)
                continue;
            return -1;
        }else if(nwritten == 0)
            continue;
        bufp += nwritten;
        nleft -= nwritten; 
    }
    return count;
}
void do_service(int conn)
{
    struct packet recvbuf;
    int n;

    while(1)
    {
        memset(&recvbuf, 0, sizeof(recvbuf));
        int ret = readn(conn,&recvbuf.len,4);
        if(ret == -1 )    //当返回0表示客户端关闭了
        {
            ERR_EXIT("read");
        }else if(ret < 4)
        {
            printf("client close\n");
            break;//退出函数，关闭该通信
        }
        n = ntohl(recvbuf.len);
        ret = readn(conn, recvbuf.buf,n);
        if(ret == -1 )    //当返回0表示客户端关闭了
        {
            ERR_EXIT("read");
        }else if(ret < n)
        {
            printf("client close\n");
            break;//退出函数，关闭该通信
        }
        fputs(recvbuf.buf,stdout);
        writen(conn,&recvbuf,4+n);
    }   
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
    while(1)
    {
        if((conn = accept(listenfd,(struct sockaddr*)&peeraddr,&peerlen)) < 0)
            ERR_EXIT("accept");
        //将ip地址转换成点分十进制的ip地址
        printf("ip=%s,port=%d\n",inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));
        //当接收到一个连接，fork一个进程
        pid = fork();   //多进程
        if(pid == -1)
            ERR_EXIT("fork");
        if(pid == 0)   //子进程 
        {   
            //子进程不需要处理监听，将监听套接口关闭
            printf("child: pid=%d\n",pid);
            close(listenfd);
            do_service(conn);
            exit(EXIT_SUCCESS);//当客户端返回时，为客户端开辟的子进程应该退出，退出子进程
        }else
        {
            printf("pid = %d\n",pid);
            //父进程不需要处理连接，关闭连接套接口
            close(conn);
        }
    }

    return 0;
}       
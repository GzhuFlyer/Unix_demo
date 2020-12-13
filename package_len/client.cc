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
    ssize_t nread;  //接收到的字节数
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
int main(void)
{
    int sock;
    if((sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // servaddr.sin_addr.s_addr = inet_addr("47.112.152.139");

    if(connect(sock,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
       ERR_EXIT("connect");
    //    printf("error connect\n");
    struct packet sendbuf;
    struct packet recvbuf;
    memset(&sendbuf,0,sizeof(sendbuf));
    memset(&recvbuf,0,sizeof(recvbuf));
    int n;
    while(fgets(sendbuf.buf,sizeof(sendbuf.buf),stdin) != NULL)
    {
       n = strlen(sendbuf.buf);
       sendbuf.len = htonl(n);
       writen(sock,&sendbuf,4+n);

       int ret = readn(sock,&recvbuf.len,4);
       if(ret == -1)
            ERR_EXIT("read");
       else if(ret < 4)
       {
           printf("client close");
           break;
       }

        n=ntohl(recvbuf.len);
        ret = readn(sock,recvbuf.buf,n);
        if(ret == -1)
            ERR_EXIT("read");
        else if(ret < n)
        {
            printf("client close\n");
            break;
        }
        fputs(recvbuf.buf,stdout);
        memset(&sendbuf,0,sizeof(sendbuf));
        memset(&recvbuf,0,sizeof(recvbuf));
    }
    close(sock);
    return 0;
}       
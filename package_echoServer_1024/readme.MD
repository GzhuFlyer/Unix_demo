#问题
#１，发送数据包的时候，应用进程缓冲区数据大小不确定
#　　有可能超过套接口缓冲区大小，从而对发送的消息进行分割
#2，TCP发送的数据包有最大段的限制，从而对消息进行分割
#3，IP层MTU大小有限制
#解决方法
#定长包，包尾加\r\n，包头加上包体的长度等等.......

#该示例目录展示发送1024个字节定长包区分消息之间的边界
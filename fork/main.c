#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>


int main(int argc, char *argv[])
{
	for(int i=0;i<3;i++)	//进行4次fork, 进程数为16,打印数为：2+4+8+16=30
	{
		pid_t pid = fork();
		if(pid < 0)
		{
			fprintf(stderr,"创建进程出错\n");
		}else if(pid == 0){
			//while(1){
				printf("子进程空间,getppid()=%d\n",getpid());
				//sleep(3);				//执行 ./a.out 后很快结束
			//}	
		}else{
			//while(1){
				printf("父进程空间，子进程pid为%d,getppid()=%d\n",pid,getpid());
				char str[10];
				char *p = str;
				fgets(p,10,stdin);	//父进程处于等待终端接收字符状态，程序执行后不会很快关闭
				//sleep(1);	
			//}
		}
		//因此在执行./a.out后可以在ps -ef　中看到三个父进程
	}

	return 0;
}

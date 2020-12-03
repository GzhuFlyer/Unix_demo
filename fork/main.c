#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>


int main(int argc, char *argv[])
{
	for(int i=0;i<4;i++)	//进行4次fork, 进程数为16,打印数为：2+4+8+16=30
	{
		pid_t pid = fork();
		if(pid < 0)
		{
			fprintf(stderr,"创建进程出错\n");
		}else if(pid == 0){
			//while(1){
				printf("子进程空间,getppid()=%d\n",getpid());
				//sleep(3);
			//}	
		}else{
			//while(1){
				printf("父进程空间，子进程pid为%d,getppid()=%d\n",pid,getpid());
				//sleep(1);	
			//}
		}
	}

	return 0;
}

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>


void test()
{
	
}

int main(int argc, char *argv[])
{
	pid_t pid = fork();
	if(pid < 0)
	{
		fprintf(stderr,"创建进程出错\n");
	}else if(pid == 0){
		while(1){
			printf("子进程空间,g_count=%d\n",g_count++);
			sleep(3);
		}	
	}else{
		while(1){
			printf("父进程空间，子进程pid为%d,g_coutn=%d\n",pid,g_count++);
			sleep(1);	
		}
	}
	return 0;
}

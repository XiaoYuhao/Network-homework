//test3-2.c
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<stdlib.h>
int main(){
	pid_t pid;
	int i,j,k;
	
	pid=fork();//�����ӽ���
	
	if(pid<0)
	{
		printf("fork error\n");
		exit(1);
	}
	else if(pid>0){
		exit(0);//�˳�������
	}
	//����ʹ�ӽ��̳�Ϊ�ػ�����
	setsid();//�����»Ự
	chdir("/");//�ı䵱ǰ����Ŀ¼Ϊ��Ŀ¼
	umask(0);//�����ļ�Ȩ������
	
	
	pid=fork();
	
	if(pid>0){
		int status=0,ret;
		while(1){
			sleep(1);
			ret=waitpid(pid,&status,WNOHANG);
			if(ret){
				printf("%d�Ž����յ���%d���ź�\n",pid,status);
				break;
			}
		}
	}
	
	else{
		while(1){
			sleep(1);
			exit(1);
		}
	}

	
	return 0;
}
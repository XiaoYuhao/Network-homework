//test3-1.c
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<stdlib.h>
#include<sys/prctl.h>
#include<string.h>
#include<time.h>
#include <stdarg.h>
#include <stdlib.h>
# define MAXLINE 2048
#include<signal.h>
extern char** environ;


void my_initproctitle(char* argv[], char** last);
void my_setproctitle(char* argv[], char** last, char* title);
int readconf(char fname[],int *num);

void showtime(int tt,char ss[]){
	int h,m,s;
	h=tt/3600;
	tt-=3600*h;
	m=tt/60;
	tt-=60*m;
	s=tt;
	sprintf(ss,"%02d:%02d:%02d",h,m,s);
}

void sub(char *argv[],char pargv[],int j){
	char buf[512];
	char runtime[20];
	time_t t_start,t_end,tt;
	char* p_last = NULL;
	t_start=time(NULL);
	while(1){
		t_end=time(NULL);
		tt=difftime(t_end,t_start);
		showtime(tt,runtime);
		sprintf(buf,"%s [sub-%02d %s] \0",pargv,j,runtime);
		my_initproctitle(argv,&p_last);
		strcpy(argv[0],buf);
		sleep(1);
		prctl(PR_SET_PDEATHSIG,SIGKILL);
	}
	return ;
}
int main(int argc,char *argv[]){
	int num;
	readconf("/etc/1652195.conf",&num);
	
	time_t pt_start,pt_end,ptt;
	pt_start=time(NULL);
	char pargv[16];
	strcpy(pargv,argv[0]);
	char* pp_last = NULL;
			
	pid_t pid[20],ret;
	int i,j,k;
		
	pid[0]=fork();
	
	if(pid[0]<0){
		printf("fork error\n");
		exit(1);
	}
	else if(pid[0]>0){
		exit(0);
	}
	
	char new_name[30];
	strcpy(new_name,argv[0]);
	strcat(new_name," [main] ");
	prctl(PR_SET_NAME,new_name);
	
	for(j=1;j<=num;j++){
		char buf[512];
		char runtime[20];
		pt_end=time(NULL);
		ptt=difftime(pt_end,pt_start);
		showtime(ptt,runtime);
		sprintf(buf,"%s [main %s] \0",pargv,runtime);
		my_initproctitle(argv,&pp_last);
		strcpy(argv[0],buf);
				
		pid[j]=fork();//创建子进程
		if(pid[j]<0){//创建子进程失败
			printf("fork error\n");
		}
		if(pid[j]==0){
			sub(argv,pargv,j);
/*			char buf[512];
			char runtime[20];
			time_t t_start,t_end,tt;
			char* p_last = NULL;
			t_start=time(NULL);
			while(1){
				t_end=time(NULL);
				tt=difftime(t_end,t_start);
				showtime(tt,runtime);
				sprintf(buf,"%s [sub-%02d %s] \0",pargv,j,runtime);
				my_initproctitle(argv,&p_last);
				strcpy(argv[0],buf);
				sleep(1);
			}
			return 0;*/
		}
		sleep(1);
	}

	while(1){
		char buf[512];
		char runtime[20];
		pt_end=time(NULL);
		ptt=difftime(pt_end,pt_start);
		showtime(ptt,runtime);
		sprintf(buf,"%s [main %s] \0",pargv,runtime);
		my_initproctitle(argv,&pp_last);
		strcpy(argv[0],buf);
		sleep(1);
		
		for(i=1;i<=num;i++){
			ret=waitpid(pid[i],NULL,WNOHANG);
			if(ret>0){
				pid[i]=fork();
				if(pid[i]==0){
					sub(argv,pargv,i);
				}
			}
		}
	}
	
	return 0;
}

void my_initproctitle(char* argv[], char** last)
{
        int i = 0;
        char* p_tmp = NULL;
        size_t i_size = 0;
 
        for(i = 0; environ[i]; i++){
                i_size += strlen(environ[i]) + 1;
        }
 
        p_tmp = (char *)malloc(i_size);
        if(p_tmp == NULL){
                return ;
        }
 
        *last = argv[0];
        for(i = 0; argv[i]; i++){
                *last += strlen(argv[i]) + 1;
        }
 
        for(i = 0; environ[i]; i++){
                i_size = strlen(environ[i]) + 1;
                *last += i_size;
 
                strncpy(p_tmp, environ[i], i_size);
                environ[i] = p_tmp;
                p_tmp += i_size;
        }
 
        (*last)--;
 
        return ;
}
 
 
void my_setproctitle(char* argv[], char** last, char* title)
{
        char* p_tmp = NULL;
        /* argv[1] = NULL; */
        p_tmp = argv[0];
        /* memset(p_tmp, 0, *last - p_tmp); */
        strncpy(p_tmp, title, *last - p_tmp);
        return ;
}

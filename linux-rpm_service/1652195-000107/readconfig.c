//lib1652195.c
#include<stdio.h>
#include <stdlib.h>
#include<string.h>
int readconf(char fname[],int *num){
	FILE *fp=fopen(fname,"r");
	if(fp==NULL){
		printf("Open file error!");
		return 0;
	}
	char buf[512];
	char snum[512];
	char ch;
	int k=0;
	fscanf(fp,"%s",buf);
	int i=0;
	while(buf[i++]!='=');
	while(buf[i]!='\0'){
		snum[k++]=buf[i++];
	}
/*	printf("hello world!\n");
	while((ch=fgetc(fp))!='='){
		printf("%c",ch);
	}
	while((ch=fgetc(fp))!='\n'){
		buf[k++]=ch;
		printf("%c",ch);
	}
	printf("\n");*/
	snum[k]='\0';
	int res=atoi(snum);
	if(res<5||res>20)res=5;
	*num=res;
	fclose(fp);
	return 1;
}
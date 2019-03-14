#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include<sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include <map>
#include <string>
#include <iostream>
#include"../common/tools.h"
using namespace std;


int main(){
	struct shared_memory* shared=get_share_memory(12);
	while(shared->ready!=1){
		sleep(1);
	}
	unsigned char buf[MAX_LEN];
	int len=shared->len-14;
	
	
	eth_hdr ethd;
	memcpy(&ethd,shared->data,14);
	ethdchange_to_host(ethd);
	show_ethd(ethd);
	unsigned short type;
	type=ethd.type;
	if(type!=0x0800){
		cout<<"Ethernet头部类型错误！"<<endl;
		exit(1);
	}
	
	memcpy(buf,&shared->data[14],len);
	
	delete_share_memory(12);
	
	
	struct shared_memory* shared2=create_share_memory(23);
	
	shared2->ready=0;
	shared2->len=len;
	memcpy(shared2->data,buf,shared2->len);
	shared2->ready=2;
	
	//printf_data_hex(shared2->data,shared2->len);
	//cout<<"send-level4-len:"<<dec<<shared2->len<<endl;
	return 0;
	
}


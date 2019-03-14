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
	struct shared_memory* shared=get_share_memory(2);
	while(shared->ready!=1){
		sleep(1);
	}

	eth_hdr ethd;
	memcpy(&ethd,shared->data,14);
	ethdchange_to_host(ethd);
	show_ethd(ethd);
	unsigned short type;
	type=ethd.type;
	cout<<"Ethernet头部类型:"<<hex<<type<<endl;
	if(type!=0x0800){
		cout<<"Ethernet头部类型错误！"<<endl;
		delete_share_memory(2);
		exit(1);
	}
	shared->ready=2;
	return 0;
	
}


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
	while(shared->ready!=2){
		sleep(1);
	}
	unsigned short cksum_recv;
	
	ip_hdr_net iph_net;
	memcpy(&iph_net,&shared->data[14],20);
	cksum_recv=iph_net.cksum;
	iph_net.cksum=0;
	
	unsigned short chkbuf[60];
	memcpy(chkbuf,&iph_net,20);
	
	iph_net.cksum=checksum(chkbuf,20);
	cout<<"接收到的IP头校验值为:    "<<hex<<setw(4)<<setfill('0')<<ntohs(cksum_recv)<<endl;
	cout<<"计算得到的IP头检验值为:  "<<hex<<setw(4)<<setfill('0')<<ntohs(iph_net.cksum)<<endl;
	if(iph_net.cksum!=cksum_recv){
		cout<<"接受到的IP头检验和计算得到的IP校验值不一致!"<<endl;
		cout<<"出错退出！"<<endl;
		delete_share_memory(2);
		exit(1);
	}
	
	ip_hdr iph;
	ipchange_to_host(iph_net,iph);//转主机序
	
	show_iph(iph);
	shared->len=14+iph.iplen;//len去掉填充字节长度
	shared->ready=3;
	return 0;
}
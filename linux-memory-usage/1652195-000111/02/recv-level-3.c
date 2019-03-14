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
	cout<<"���յ���IPͷУ��ֵΪ:    "<<hex<<setw(4)<<setfill('0')<<ntohs(cksum_recv)<<endl;
	cout<<"����õ���IPͷ����ֵΪ:  "<<hex<<setw(4)<<setfill('0')<<ntohs(iph_net.cksum)<<endl;
	if(iph_net.cksum!=cksum_recv){
		cout<<"���ܵ���IPͷ����ͼ���õ���IPУ��ֵ��һ��!"<<endl;
		cout<<"�����˳���"<<endl;
		delete_share_memory(2);
		exit(1);
	}
	
	ip_hdr iph;
	ipchange_to_host(iph_net,iph);//ת������
	
	show_iph(iph);
	shared->len=14+iph.iplen;//lenȥ������ֽڳ���
	shared->ready=3;
	return 0;
}
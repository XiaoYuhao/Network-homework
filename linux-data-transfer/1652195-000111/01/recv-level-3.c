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
	struct shared_memory* shared=get_share_memory(23);
	while(shared->ready!=2){
		sleep(1);
	}
	unsigned char buf[MAX_LEN];
	int len=shared->len;
	
	unsigned short cksum_recv;
	
	ip_hdr_net iph_net;
	memcpy(&iph_net,shared->data,20);
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
		exit(1);
	}
	
	ip_hdr iph;
	ipchange_to_host(iph_net,iph);//ת������
	
	show_iph(iph);

	memcpy(buf,&shared->data,len);//��IPҲ���봫��㣬��Ϊ����TCPУ��ֵ��Ҫʹ�õ�
	delete_share_memory(23);
	
	struct shared_memory* shared2=create_share_memory(34);
	shared2->ready=0;
	shared2->len=iph.iplen;//ֻҪIPͷ��iplen���ȵ����ݣ�������ֽ�ȥ��
	memcpy(shared2->data,buf,shared2->len);
	shared2->ready=3;
	//printf_data_hex(shared2->data,shared2->len);
	//cout<<"send-level3-len:"<<dec<<shared2->len<<endl;
	return 0;
}
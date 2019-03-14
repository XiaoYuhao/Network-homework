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
	while(shared->ready!=3){
		sleep(1);
	}
	int len=shared->len;
	unsigned short cksum_recv;
	
	ip_hdr iph;
	memcpy(&iph,&shared->data[14],20);//��ȡIPͷ
	unsigned int src_ip=iph.srcip;
	unsigned int dst_ip=iph.dstip;
	
	tcp_hdr tcph;
	tcp_hdr_net tcph_net;
	//printf_data_hex(shared->data,shared->len);
	memcpy(&tcph_net,&shared->data[34],20);//��ȡTCPͷ
	tcpchange_to_host(tcph_net,tcph);
	
	int extern_len=(tcph.offset-5)*4;
	unsigned char externch[60];
	memcpy(&externch,&shared->data[54],extern_len);//��ȡTCPѡ����Ϣ
	//memcpy(&tcph,&shared->data[20],20);
	
	cksum_recv=tcph_net.cksum;
	tcph_net.cksum=0;
	unsigned char chkbuf[MAX_LEN];
	//
	memcpy(chkbuf,&tcph_net,20);
	memcpy(&chkbuf[20],&shared->data[54],len-54);
	tcph_net.cksum=tcp_chksum(chkbuf,len-34,&src_ip,&dst_ip);
	cout<<"���յ���TCPͷУ��ֵΪ:    "<<hex<<setw(4)<<setfill('0')<<ntohs(cksum_recv)<<endl;
	cout<<"����õ���TCPͷ����ֵΪ:  "<<hex<<setw(4)<<setfill('0')<<ntohs(tcph_net.cksum)<<endl;
	if(tcph_net.cksum!=cksum_recv){
		cout<<"���ܵ���TCPͷ����ͼ���õ���TCPУ��ֵ��һ��!"<<endl;
		cout<<"�����˳���"<<endl;
		delete_share_memory(2);
		exit(1);
	}
	
	show_tcph(tcph,externch,extern_len);
	shared->datalen=len-14-20-20-extern_len;
	shared->len=extern_len;
	shared->ready=4;
	return 0;
}
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
	struct shared_memory* shared=get_share_memory(43);
	while(shared->ready!=4){
		sleep(1);
	}
	unsigned char buf3[MAX_LEN];
	int len=shared->len;
	
	
	map<string,string> m_mapConfigInfo;
	ConfigFileRead(m_mapConfigInfo);//读取配置文件
	
	ip_hdr iph;
	iph.len_ver=0x45;
	iph.tos=0x00;
	iph.iplen=len+20;//总长度
	iph.identification=rand()%65536;
	iph.flag=strtol(m_mapConfigInfo["ip_flag"].c_str(),NULL,2);
	if(m_mapConfigInfo.count("ip_offset")){
		iph.offset=atoi(m_mapConfigInfo["ip_offset"].c_str());
		if(iph.offset<0||iph.offset>8191)
			iph.offset=0;
	}
	else{//缺失则为0
		iph.offset=0;
	}
	if(m_mapConfigInfo.count("ip_ttl")){
		iph.ttl=atoi(m_mapConfigInfo["ip_ttl"].c_str());
		if(iph.ttl<0||iph.ttl>255)
			iph.ttl=64;
	}
	else{//缺失则为64
		iph.ttl=64;
	}
	iph.proto=6;//TCP协议
	iph.cksum=0;
	iph.srcip=read_ip_addr(m_mapConfigInfo["ip_srcip"]);
	iph.dstip=read_ip_addr(m_mapConfigInfo["ip_dstip"]);
	
	//show_iph(iph);
	
	ip_hdr_net iph_net;
	ipchange_to_net(iph,iph_net);//转网络序
	unsigned short buff[60];
	memcpy(buff,&iph_net,20);

	//cout<<endl;
	//unsigned char buf[20];
	//memcpy(buf,&iph,20);
	//printf_data_hex(buf,20);
	iph_net.cksum=checksum(buff,20);
	iph.cksum=ntohs(iph_net.cksum);
	
	show_iph(iph);

	memcpy(buf3,&iph_net,20);
	memcpy(&buf3[20],shared->data,len);
	delete_share_memory(43);
	
	struct shared_memory* shared2=create_share_memory(32);
	shared2->ready=0;
	shared2->len=len+20;
	memcpy(shared2->data,buf3,shared2->len);
	shared2->ready=3;
	//printf_data_hex(shared2->data,shared2->len);
	//cout<<"send-level3-len:"<<dec<<shared2->len<<endl;
	return 0;
}
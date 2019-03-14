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
	struct shared_memory* shared=get_share_memory(54);
	while(shared->ready!=5){
		sleep(1);
	}
	//char buf1[MAX_LEN];
	unsigned char buf2[MAX_LEN];
	int len=shared->len;
	
	//充加入TCP头、计算校验
	srand(time(0));
	
	map<string,string> m_mapConfigInfo;
	ConfigFileRead(m_mapConfigInfo);//读取配置文件
	string temp;
	
	tcp_hdr tcph;
	if(m_mapConfigInfo.count("tcp_srcport")){
		tcph.srcport=atoi(m_mapConfigInfo["tcp_srcport"].c_str());
	}
	else{//缺失则随机产生
		tcph.srcport=rand()%65535+1;
	}
	if(m_mapConfigInfo.count("tcp_dstport")){
		tcph.dstport=atoi(m_mapConfigInfo["tcp_dstport"].c_str());
	}
	else{//缺失则为80
		tcph.dstport=80;
	}
	tcph.seq=rand();
	tcph.ack=rand();
	tcph.offset=atoi(m_mapConfigInfo["tcp_offset"].c_str());
	tcph.reserved=0;
	tcph.flag=strtol(m_mapConfigInfo["tcp_flag"].c_str(),NULL,2);
	tcph.window=rand()%65536;
	if(tcph.flag&32){//URG位为1
		tcph.urg=rand()%65535+1;
	}
	else{
		tcph.urg=0;
	}
	tcph.cksum=0;
	
	tcp_hdr_net tcph_net;
	
	tcpchange_to_net(tcph,tcph_net);
	unsigned char buf[MAX_LEN];//最大开60
	memcpy(buf,&tcph_net,20);//
	
	unsigned char externch[40];
	int extern_len=0;
	if(tcph.offset>5){
		extern_len=(tcph.offset-5)*4;
		for(int i=0;i<extern_len;i++){
			externch[i]=rand()%256;
		}
		memcpy(&buf[20],externch,extern_len);//添加tcp选项
	}
	
	
	string src_ip=m_mapConfigInfo["ip_srcip"];
	string dst_ip=m_mapConfigInfo["ip_dstip"];
	unsigned int srcaddr=htonl(read_ip_addr(src_ip));
	unsigned int dstaddr=htonl(read_ip_addr(dst_ip));
	memcpy(&buf[20+extern_len],shared->data,len);
	tcph_net.cksum=tcp_chksum(buf,20+extern_len+len,&srcaddr,&dstaddr);
	//cout<<"TCP报头+数据:"<<endl;
    //printf_data_hex(buf,20+extern_len+len);
	//unsigned char hostbuf[MAX_LEN];
	//memcpy(hostbuf,&tcph,20);
	//cout<<"tpc_head_host"<<endl;
	//printf_data_hex(hostbuf,20);

	tcph.cksum=ntohs(tcph_net.cksum);
	
	show_tcph(tcph,externch,extern_len);
	
	unsigned char buf1[60];
	memcpy(buf1,&tcph_net,20);
	if(extern_len>0)memcpy(&buf1[20],externch,extern_len);
	memcpy(buf2,buf1,20+extern_len);
	memcpy(buf2+20+extern_len,shared->data,len);
	delete_share_memory(54);
	
	
	struct shared_memory* shared2=create_share_memory(43);
	
	shared2->ready=0;
	shared2->len=len+20+extern_len;
	memcpy(shared2->data,buf2,shared2->len);
	shared2->ready=4;
	
	//printf_data_hex(shared2->data,shared2->len);
	//cout<<"send-level4-len:"<<dec<<shared2->len<<endl;
	return 0;
	
}


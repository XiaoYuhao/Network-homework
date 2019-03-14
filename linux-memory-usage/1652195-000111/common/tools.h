#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
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
#include <fstream>
#include <iostream>
#include <iomanip>
#include <bitset>
#include "tcp_ip.h"
using namespace std;

const char* conf_file="network.conf";

void perror_exit(const char *hint)
{
    printf("Function [%s] failed: %s (errno: %d)\n",hint,strerror(errno),errno);
    exit(1);    
}

struct shared_memory
{
	int written1;
	int written2;
	unsigned char data[MAX_LEN];
	int len;
	int ready;
	int datalen;
};

struct shared_memory* create_share_memory(int id){
	int shmid;

	if((shmid = shmget((key_t)id, sizeof(struct shared_memory), 0666|IPC_CREAT)) < 0)
		perror_exit("shmget");

	void* shm;

	if((shm = shmat(shmid, 0, 0)) == (void*)-1)
		perror_exit("shmat");

	struct shared_memory *shared = (struct shared_memory*)shm;

	shared->written1=0;
	shared->written2=0;
	return shared;
	
}

struct shared_memory* get_share_memory(int id){
	int shmid;

	if((shmid = shmget((key_t)id, sizeof(struct shared_memory), 0666|IPC_CREAT)) < 0)
		perror_exit("shmget");

	void* shm;

	if((shm = shmat(shmid, 0, 0)) == (void*)-1)
		perror_exit("shmat");

	struct shared_memory *shared = (struct shared_memory*)shm;

	return shared;
}

void delete_share_memory(int id)
{
	int shmid;

	if((shmid = shmget((key_t)id, sizeof(struct shared_memory), 0666|IPC_CREAT)) < 0)
		perror_exit("shmget");

	void* shm;

	if((shm = shmat(shmid, 0, 0)) == (void*)-1)
		perror_exit("shmat");

	struct shared_memory *shared = (struct shared_memory*)shm;

	if(shmdt(shared)==-1){
		perror("shmdt");
		exit(1);
	}
	
	if(shmctl(shmid,IPC_RMID,0)==-1){
		perror("shmctl");
		exit(1);
	}
}

void ConfigFileRead(map<string,string>&m_mapConfigInfo){
	ifstream configFile;
	configFile.open(conf_file);
	string str_line;
	
	string perfix;
	
	if(configFile.is_open()){
		while(!configFile.eof()){
			getline(configFile,str_line);
			if(str_line.find('#')==0)continue;
			else if(str_line.size()==0)continue;
			else if(str_line.find("[应用层]")==0){
				perfix="use_";
			}
			else if(str_line.find("[传输层]")==0){
				perfix="tcp_";
			}
			else if(str_line.find("[网络层]")==0){
				perfix="ip_";
			}
			else if(str_line.find("[数据链路层]")==0){
				perfix="link_";
			}
			else{
				size_t pos=str_line.find('=');
				size_t pos2=str_line.find('#');
				string str_key=str_line.substr(0,pos);
				string str_value;
				if(pos2<0){
					str_value=str_line.substr(pos+1);
				}
				else{
					str_value=str_line.substr(pos+1,pos2-pos-1);
				}
				str_key=perfix+str_key;
				m_mapConfigInfo.insert(pair<string,string>(str_key,str_value));
			}
		}
	}
	else{
		cout<<"Cannot not open config file:"<<conf_file<<endl;
		exit(-1);
	}
}

void write_dat_file_hex(const char * file_name,const unsigned char *buf,const int len){
	ofstream file;
	file.open(file_name);
	
	if(file.is_open()){
		for(int i=0;i<len;i++){
			if(i!=0&&i%16==0)file<<endl;
			file<<hex<<setw(2)<<setfill('0')<<int(buf[i])<<" ";
		}
	}
	else{
		cout<<"Cannot open this file"<<file_name<<endl;
		exit(-1);
	}
}
void read_dat_file_hex(const char *file_name,unsigned char *buf,int *len){
	ifstream file;
	file.open(file_name);
	
	int kk;
	if(file.is_open()){
		int i=0;
		while(file>>hex>>kk){
			//file>>hex>>kk;
			buf[i]=kk;
			i++;
		}
		*len=i;
	}
	else{
		cout<<"Cannot open this file"<<file_name<<endl;
		exit(-1);
	}
}

unsigned int read_ip_addr(string s) {
	unsigned int rst = 0;
	int temp1, temp2, temp3, temp4;
	string temp;
	int k = 3;
	for (int i = 0; i < s.size(); i++) {
		if (s[i] == '.') {
			int n = atoi(temp.c_str());
			rst += n << k * 8;
			k--;
			temp.clear();
		}
		else {
			temp += s[i];
		}
	}
	rst += atoi(temp.c_str());
	return rst;
}

void read_mac_addr(string s, unsigned char *buf) {
	string temp;
	int i, k = 0;
	for (i = 0; i < s.size(); i++) {
		if (s[i] == ':') {
			buf[k++] = strtol(temp.c_str(), NULL, 16);
			temp.clear();
		}
		else {
			temp += s[i];
		}
	}
	buf[k] = strtol(temp.c_str(), NULL, 16);
}

void printf_data_hex(const unsigned char *buf,const int len){
	for(int i=0;i<len;i++){
		if(i!=0&&i%16==0)cout<<endl;
		cout<<hex<<setw(2)<<setfill('0')<<int(buf[i])<<" ";
	}
	cout<<endl;
}

void write_ip_addr(const unsigned int num, unsigned char * buf) {
	unsigned int t[4] = { 0xff000000 ,0x00ff0000 ,0x0000ff00 ,0x000000ff };
	unsigned int temp;
	int k = 0;
	for (int i = 0; i < 4; i++) {
		temp = t[i] & num;
		buf[i] = temp >> (8 * (3 - i));
	}
}

void show_iph(ip_hdr iph){
	cout<<endl;
	cout<<"IP包头信息:"<<endl;
	cout<<"ver+len="<<hex<<int(iph.len_ver)<<endl;
	cout<<"tos="<<int(iph.tos)<<endl;
	cout<<"iplen="<<hex<<setw(4)<<setfill('0')<<int(iph.iplen)<<'('<<dec<<int(iph.iplen)<<')'<<endl;
	cout<<"id="<<hex<<setw(4)<<setfill('0')<<int(iph.identification)<<endl;
	cout<<"frag="<<int(iph.flag)<<endl;
	cout<<"offset="<<int(iph.offset)<<endl;
	cout<<"ttl="<<hex<<int(iph.ttl)<<'('<<dec<<int(iph.ttl)<<')'<<endl;
	cout<<"proto="<<hex<<int(iph.proto)<<endl;
	cout<<"ckusm="<<hex<<int(iph.cksum)<<endl;
	
	cout<<"srcip="<<hex<<int(iph.srcip)<<" - ";
	unsigned char buf1[4];
	write_ip_addr(iph.srcip,buf1);
	for(int i=0;i<4;i++){
		cout<<dec<<int(buf1[i]);
		if(i!=3)cout<<'.';
	}
	cout<<endl;
	
	cout<<"dstip="<<hex<<int(iph.dstip)<<" - ";
	unsigned char buf2[4];
	write_ip_addr(iph.dstip,buf2);
	for(int i=0;i<4;i++){
		cout<<dec<<int(buf2[i]);
		if(i!=3)cout<<'.';
	}
	cout<<endl;
	cout<<endl;
}

void show_tcph(tcp_hdr tcph,unsigned char *buf,int len){
	cout<<endl;
	cout<<"TCP包头基本信息:"<<endl;
	cout<<"sport="<<hex<<tcph.srcport<<'('<<dec<<tcph.srcport<<')'<<endl;
	cout<<"dport="<<hex<<tcph.dstport<<'('<<dec<<tcph.dstport<<')'<<endl;
	cout<<"seq="<<hex<<tcph.seq<<'('<<dec<<tcph.seq<<"d)"<<endl;
	cout<<"ack="<<hex<<tcph.ack<<'('<<dec<<tcph.ack<<"d)"<<endl;
	cout<<"offset="<<int(tcph.offset)<<'('<<int(tcph.offset)*4<<')'<<endl;
	cout<<"reserved="<<bitset<6>(tcph.reserved)<<"(bit)"<<endl;
	cout<<"code="<<bitset<6>(tcph.flag)<<"(bit)"<<endl;
	cout<<"window="<<hex<<tcph.window<<'('<<dec<<tcph.window<<')'<<endl;
	cout<<"cksum="<<hex<<setw(4)<<setfill('0')<<tcph.cksum<<endl;
	cout<<"urgptr="<<hex<<setw(4)<<setfill('0')<<tcph.urg<<endl;
	cout<<endl;
	cout<<"TCP包头选项信息："<<endl;
	for(int i=0;i<len;i++){
		if(i%16==0&&i!=0)cout<<endl;
		cout<<hex<<setw(2)<<setfill('0')<<int(buf[i])<<" ";
	}
	cout<<endl;
	cout<<endl;
}

void show_ethd(eth_hdr ethd){
	cout<<endl;
	cout<<"以太网包头信息："<<endl;
	cout<<"DstMAC=";
	for(int i=0;i<6;i++){
		cout<<hex<<setw(2)<<setfill('0')<<int(ethd.dstmac[i]);
		if(i!=5)cout<<':';
	}
	cout<<endl;
	cout<<"SrcMAC=";
	for(int i=0;i<6;i++){
		cout<<hex<<setw(2)<<setfill('0')<<int(ethd.srcmac[i]);
		if(i!=5)cout<<':';
	}
	cout<<endl;
	cout<<"Type="<<hex<<setw(4)<<setfill('0')<<ethd.type<<"(IP)"<<endl;
	cout<<endl;
}

void ipchange_to_net(ip_hdr iph,ip_hdr_net &iph_net){
	iph_net.len_ver=iph.len_ver;
	iph_net.tos=iph.tos;
	iph_net.iplen=htons(iph.iplen);
	iph_net.identification=htons(iph.identification);
	//iph.flag=htonl(iph.flag);
	unsigned short temp=0;
	temp+=iph.flag<<13;
	temp+=iph.offset;
	iph_net.offset_flag=htons(temp);
	iph_net.ttl=iph.ttl;
	iph_net.proto=iph.proto;
	iph_net.cksum=htons(iph.cksum);
	iph_net.srcip=htonl(iph.srcip);
	iph_net.dstip=htonl(iph.dstip);
	
}

void ipchange_to_host(ip_hdr_net iph_net,ip_hdr &iph){
	iph.len_ver=iph_net.len_ver;
	iph.tos=iph_net.tos;
	iph.iplen=ntohs(iph_net.iplen);
	iph.identification=ntohs(iph_net.identification);
	//iph.flag=htonl(iph.flag);
	unsigned short temp;
	temp=ntohs(iph_net.offset_flag);
	iph.flag=temp>>13;
	iph.offset=temp&0x1fff;
	iph.ttl=iph_net.ttl;
	iph.proto=iph_net.proto;
	iph.cksum=ntohs(iph_net.cksum);
	iph.srcip=ntohl(iph_net.srcip);
	iph.dstip=ntohl(iph_net.dstip);
	
	
}

void tcpchange_to_net(tcp_hdr tcph,tcp_hdr_net &tcph_net){
	tcph_net.srcport=htons(tcph.srcport);
	tcph_net.dstport=htons(tcph.dstport);
	tcph_net.seq=htonl(tcph.seq);
	tcph_net.ack=htonl(tcph.ack);
	unsigned short temp=0;
	temp+=tcph.offset<<12;
	temp+=tcph.reserved<<6;
	temp+=tcph.flag;
	tcph_net.orf=htons(temp);
	tcph_net.window=htons(tcph.window);
	tcph_net.cksum=htons(tcph.cksum);
	tcph_net.urg=htons(tcph.urg);
}

void tcpchange_to_host(tcp_hdr_net tcph_net,tcp_hdr &tcph){
	tcph.srcport=ntohs(tcph_net.srcport);
	tcph.dstport=ntohs(tcph_net.dstport);
	tcph.seq=ntohl(tcph_net.seq);
	tcph.ack=ntohl(tcph_net.ack);
	unsigned short temp;
	temp=htons(tcph_net.orf);
	tcph.offset=temp>>12;
	tcph.reserved=(temp>>6)&0x3f;
	tcph.flag=temp&0x3f;
	tcph.window=ntohs(tcph_net.window);
	tcph.cksum=ntohs(tcph_net.cksum);
	tcph.urg=ntohs(tcph_net.urg);
}

void ethdchange_to_net(eth_hdr &ethd){
	ethd.type=htons(ethd.type);
}

void ethdchange_to_host(eth_hdr &ethd){
	ethd.type=ntohs(ethd.type);
}


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
#include <string>
#include <map>
#include<iostream>
#include"../common/tools.h"
using namespace std;

const char* sender_file="sender.dat";

int main(){
	int num;

	map<string,string> m_mapConfigInfo;
	ConfigFileRead(m_mapConfigInfo);
	if(m_mapConfigInfo.count("use_datalen")){
		num=atoi(m_mapConfigInfo["use_datalen"].c_str());
	}
	else{//缺失则为1024
		num=1024;
	}
/*	map<string,string>::iterator iter;
	iter=m_mapConfigInfo.begin();
	while(iter!=m_mapConfigInfo.end()){
		cout<<iter->first<<"="<<iter->second<<endl;
		iter++;
	}*/

	unsigned char data[MAX_LEN];
	int i;
	srand(time(0));
	for(i=0;i<num;i++){
		data[i]=rand()%256;
	}
	cout<<"数据:"<<endl;
	printf_data_hex(data,num);
	cout<<"已写入文件:sender.dat"<<endl;
	
	write_dat_file_hex("sender.dat",data,num);
	
	struct shared_memory* shared=create_share_memory(54);//在应用层和网络层之间创建共享内存
	shared->ready=0;
	memcpy(shared->data,data,num);//裸数据
	shared->len=num;//裸数据长度
	shared->ready=5;//应用层已经发送数据
	//cout<<"send-level5-len:"<<dec<<shared->len<<endl;
	return 0;
}
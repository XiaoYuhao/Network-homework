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
	else{//ȱʧ��Ϊ1024
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
	cout<<"����:"<<endl;
	printf_data_hex(data,num);
	cout<<"��д���ļ�:sender.dat"<<endl;
	
	write_dat_file_hex("sender.dat",data,num);
	
	struct shared_memory* shared=create_share_memory(54);//��Ӧ�ò�������֮�䴴�������ڴ�
	shared->ready=0;
	memcpy(shared->data,data,num);//������
	shared->len=num;//�����ݳ���
	shared->ready=5;//Ӧ�ò��Ѿ���������
	//cout<<"send-level5-len:"<<dec<<shared->len<<endl;
	return 0;
}
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
	struct shared_memory* shared=get_share_memory(1);
	while(shared->ready!=3){
		sleep(1);
	}
	unsigned char buf2[MAX_LEN];
	int len=shared->len;
	
	
	map<string,string> m_mapConfigInfo;
	ConfigFileRead(m_mapConfigInfo);//读取配置文件
	
	eth_hdr ethd;
	read_mac_addr(m_mapConfigInfo["link_dstmac"],ethd.dstmac);
	read_mac_addr(m_mapConfigInfo["link_srcmac"],ethd.srcmac);
	
	ethd.type=0x0800;
	
	show_ethd(ethd);
	
	ethdchange_to_net(ethd);
	memcpy(buf2,&ethd,14);
	
	//printf_data_hex(buf2,14);
	
	//memcpy(&buf2[14],shared->data,len);
	//delete_share_memory(32);
	
	shared->ready=0;
	shared->len=len+14;
	if(shared->len<60)shared->len=60;//不够60字节，需要用填充字节填满
	memcpy(shared->data,buf2,14);
	shared->ready=2;
	//printf_data_hex(shared2->data,shared2->len);
	//cout<<"send-level2-len:"<<dec<<shared2->len<<endl;
	return 0;
}
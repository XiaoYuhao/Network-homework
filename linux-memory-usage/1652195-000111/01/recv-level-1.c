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

const char *shared_file="test_lock";

int main(){
	unsigned char data[MAX_LEN];
	int len;
	read_dat_file_hex("network.dat",data,&len);
	
	cout<<"读取到的数据:"<<endl;
	printf_data_hex(data,len);
	
	
	struct shared_memory* shared=create_share_memory(12);
	shared->ready=0;
	memcpy(shared->data,data,len);
	shared->len=len;
	shared->ready=1;
	
	return 0;
	
}

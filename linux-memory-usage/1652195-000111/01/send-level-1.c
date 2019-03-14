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
	struct shared_memory* shared=get_share_memory(21);
	while(shared->ready!=2){
		sleep(1);
	}
	
	printf_data_hex(shared->data,shared->len);
	cout<<"已写入文件:network.dat"<<endl;
	write_dat_file_hex("network.dat",shared->data,shared->len);

	delete_share_memory(21);
	return 0;
	
}
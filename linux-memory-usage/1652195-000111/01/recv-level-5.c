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
	struct shared_memory* shared=get_share_memory(45);
	while(shared->ready!=4){
		sleep(1);
	}
	
	printf_data_hex(shared->data,shared->len);
	cout<<"以写入文件:receiver.dat"<<endl;
	write_dat_file_hex("receiver.dat",shared->data,shared->len);
	
	delete_share_memory(45);
	return 0;
}
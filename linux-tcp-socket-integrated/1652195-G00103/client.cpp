

#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<iostream> 
#include<sys/shm.h>           
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include <sys/prctl.h>
#include<sys/wait.h>
#include<string>
#include<time.h>
#include<signal.h>
#include <errno.h>
#define MAX 1024             //MAX_client_num
#define MAXLEN 100000
using namespace std;

//Used StuNo
const int no = 1652195;

//client_info
typedef struct
{
	int fd;
	int visit;
	int state;
	int connect;
	int finish;
	char sno[4];
	char pid[4];
	char time[20];
	char ran[MAXLEN];
	int rn;
} info;

info client[MAX];

int maxfd;


//tool gettime
string zero_fill(int x)
{
	string ans;
	char buf[100];
	if (x >= 10)
		sprintf(buf, "%d", x);
	else
		sprintf(buf, "0%d", x);
	ans = buf;
	return ans;
}

void format_time_basic(time_t time1, char *szTime)
{
	struct tm tm1;
	tm1 = *localtime(&time1);

	sprintf(szTime, "%4d-%s-%s %s:%s:%s",
		tm1.tm_year + 1900, (char *)zero_fill(tm1.tm_mon + 1).data(), (char *)zero_fill(tm1.tm_mday).data(),
		(char *)zero_fill(tm1.tm_hour).data(), (char *)zero_fill(tm1.tm_min).data(), (char *)zero_fill(tm1.tm_sec).data());
}

string format_time()
{
	string ans;
	char raw[25];
	time_t curtime = time(0);
	format_time_basic(curtime, raw);
	ans = raw;
	return ans;
}
//tool end


void timeout(int sig)
{
	exit(1);
}

int get_parameter(int argc, char **argv, char *ip, int *port, int *isblock, int *isfork, int *connect_num)
{

	int i;
	int flags[5];
	memset(flags, 0, sizeof(flags));
	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--ip") == 0)
		{
			if (flags[0] || i >= argc - 1)
				return -1;
			flags[0] = 1;
			strcpy(ip, argv[i + 1]);
		}
		if (strcmp(argv[i], "--port") == 0)
		{
			if (flags[1] || i >= argc - 1)
				return -1;
			flags[1] = 1;
			*port = atoi(argv[i + 1]);
		}
		if (strcmp(argv[i], "--block") == 0 || strcmp(argv[i], "--nonblock") == 0)
		{
			if (flags[2])
				return -1;
			if (strcmp(argv[i], "--block") == 0)
				*isblock = 1;
			else
				*isblock = 0;
			flags[2] = 1;
		}
		if (strcmp(argv[i], "--fork") == 0 || strcmp(argv[i], "--nofork") == 0)
		{
			if (flags[3])
				return -1;
			if (strcmp(argv[i], "--fork") == 0)
				*isfork = 1;
			else
				*isfork = 0;
			flags[3] = 1;
		}
		if (strcmp(argv[i], "--num") == 0)
		{
			if (flags[4] || i >= argc - 1)
				return -1;
			*connect_num = atoi(argv[i + 1]);
			if (*connect_num < 1 || *connect_num>1000)
				return -1;
			flags[4] = 1;
		}
	}

	for (i = 0; i < 5; i++)
	{
		if (i <= 1)
		{
			if (flags[i] == 0)
				return -1;
		}
		else
		{
			if (flags[i] == 0)
			{
				switch (i)
				{
				case 2:
					*isblock = 0;
					break;
				case 3:
					*isfork = 0;
					break;
				case 4:
					*connect_num = 100;
				default:
					break;
				}
			}
		}

	}
	if (*isfork == 0 && *isblock == 1)    //--nofork 与--block 同时出现时，--block 无效
		*isblock = 0;
	return 0;
}


int initialize(int isblock)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket error ");
		return -1;
	}
	if (isblock == 0)
	{
		int val;
		if (val = fcntl(sock, F_GETFL, 0) < 0) {//获取文件状态标志
			perror("fcntl");
			close(sock);
			return -1;
		}
		if (fcntl(sock, F_SETFL, val | O_NONBLOCK) < 0) {//设置文件状态标志
			perror("fcntl");
			close(sock);
			return -1;
		}
	}
	return sock;
}


int myconnect(int sockfd, char *ip, int port, int isblock)
{
	struct sockaddr_in serv_addr;
	bzero((char *)&serv_addr, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);   //不能使用htonl函数
	serv_addr.sin_port = htons(port);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0)
	{
		if (isblock)
		{
			perror("connect error");
			//close(sockfd);              //?
			return -1;
		}
		else
		{
			if (errno != EINPROGRESS) {
				perror("connect error");
				return -1;
			}
			else
			{
				fd_set wfd;
				struct timeval tm;
				FD_ZERO(&wfd);
				FD_SET(sockfd, &wfd);
				tm.tv_sec = 1; // 秒
				tm.tv_usec = 0; // 微秒
				int sel = select(sockfd + 1, NULL, &wfd, NULL, &tm);
				if (sel <= 0)
				{
					perror("connect error");
					return -1;
				}
				return 0;
			}
		}
	}
	else
	{
		if (isblock)
			return 0;
		else
		{
			perror("connect error");
			return -1;
		}
	}
}


int data_trans_block(int sockfd)                //memcpy !!!
{
	char wbuf[MAXLEN], rbuf[MAXLEN];
	int i, sj = 0;
	int wlen, rlen;
	string ref;
	char send_set[10][MAXLEN];

	int ss = 0;

	//Read StuNo             1
	memset(rbuf, 0, sizeof(rbuf));
	ref = "StuNo";
	rlen = read(sockfd, rbuf, sizeof(rbuf));
	if (rlen != ref.length())
		return -1;
	for (i = 0; i < rlen; i++)
	{
		if (rbuf[i] != ref[i])
			return -1;
	}
	
	//Write sno(1652195)            2
	memset(wbuf, 0, sizeof(wbuf));
	int neti = htonl(no);
	memcpy(wbuf, (char *)&neti, sizeof(int));           //Must memcpy Here 不能是strncpy
	wlen = write(sockfd, wbuf, sizeof(int));
	if (wlen != sizeof(int))
		return -1;
	memcpy(send_set[sj++], wbuf, sizeof(int));   //Load send_set

	//Read pid                     3
	memset(rbuf, 0, sizeof(rbuf));
	ref = "pid";
	rlen = read(sockfd, rbuf, sizeof(rbuf));
	if (rlen != ref.length())
		return -1;
	for (i = 0; i < rlen; i++)
	{
		if (rbuf[i] != ref[i])
			return -1;
	}
	
	//(fork)Write pid(num)             4
	memset(wbuf, 0, sizeof(wbuf));
	neti = htonl(getpid());
	memcpy(wbuf, (char *)&neti, sizeof(int));
	wlen = write(sockfd, wbuf, sizeof(int));
	if (wlen != sizeof(int))
		return -1;
	memcpy(send_set[sj++], wbuf, sizeof(int));   //Load send_set
	
	//Read TIME\0                   5
	memset(rbuf, 0, sizeof(rbuf));
	ref = "TIME";
	rlen = read(sockfd, rbuf, sizeof(rbuf));
	if (rlen != ref.length() + 1)   //尾0
		return -1;
	for (i = 0; i < rlen; i++)
	{
		if (rbuf[i] != ref[i])
			return -1;
	}


	//Write SystemTime                   6
	memset(wbuf, 0, sizeof(wbuf));
	strcpy(wbuf, (char *)format_time().data());
	wlen = write(sockfd, wbuf, strlen(wbuf));
	if (wlen != 19)
		return -1;
	strncpy(send_set[sj++], wbuf, strlen(wbuf));   //Load send_set

	
	//Read RandomNumber(str)
	memset(rbuf, 0, sizeof(rbuf));
	rlen = read(sockfd, rbuf, sizeof(rbuf));
	if (rlen != 8 + 1)   //尾0
		return -1;
	ref = "str";
	for (i = 0; i < 3; i++)   //前三位对应
	{
		if (rbuf[i] != ref[i])
			return -1;
	}
	int rn = atoi(&rbuf[3]);
	if (rn < 32768 || rn>99999)
		return -1;
	

	//Write randomstr  尾0 issues   
	srand(time(0));
	memset(wbuf, 0, sizeof(wbuf));
	for (i = 0; i < rn; i++)    //length rn
	{
		wbuf[i] = rand() % 256;
	}
	wlen = write(sockfd, wbuf, rn);
	if (wlen != rn)
		return -1;
	memcpy(send_set[sj++], wbuf, rn);  //use memcpy

	
	memset(rbuf, 0, sizeof(rbuf));
	ref = "end";
	rlen = read(sockfd, rbuf, sizeof(rbuf));
	if (rlen != ref.length())
		return -1;
	for (i = 0; i < rlen; i++)
	{
		if (rbuf[i] != ref[i])
			return -1;
	}
	
	close(sockfd);   //关闭连接
	char filename[100];
	sprintf(filename, "txt/%d.%d.pid.txt", no, getpid());
	int file = open(filename, O_CREAT | O_TRUNC | O_WRONLY);
	int len_set[] = { 4,4,19,rn };           //写入长度
	for (i = 0; i < sj; i++)
	{
		write(file, send_set[i], len_set[i]);
		write(file, "\n", 1);
	}
	close(file);
	/*FILE *fp = fopen(filename, "w");
	int len_set[] = { 4,4,19,5 };
	for (i = 0; i < sj; i++)
	{
		fwrite(send_set[i], 1, len_set[i], fp);
		fwrite("\n", 1, 1, fp);
	}
	fclose(fp);*/
	return 0;

}



int block_process(char *ip, int port, int connect_num)                   //fork  no select
{
	//signal(SIGCHLD, SIG_IGN);
	int num = connect_num;
	pid_t pid;
	int status = 0, ret = 0;
	int success_num = 0;
	printf("总通信client数为:%d\n", connect_num);
	while (1) {
		if (num <= 0) {//全部fork已完成
			ret = waitpid(-1, &status, WNOHANG);
			if (ret > 0) {//有子进程结束了
				//printf("回收进程pid:%d\n", ret);
				if (WIFEXITED(status)) {//子进程是被exit或return结束的
					if (WEXITSTATUS(status) == 1) {//exit或return值为1
						//printf("end\n");
						num++;
					}
					else  if(WEXITSTATUS(status) == 0){
						//exit或return值为0，则表示成功结束
						success_num++;
						printf("已成功通信client个数:%d\n", success_num);
						if (success_num == connect_num) {//成功通信的个数等于总个数
							return 1;
						}
					}
				}
				else {//子进程异常结束
					printf("%d进程异常退出,收到信号:%d\n", pid, status);
					num++;
				}
			}
		}
		else {
			pid = fork();//分裂子进程
			if (pid < 0) {
				perror("fork");
				continue;
			}
			if (pid == 0) {
				prctl(PR_SET_PDEATHSIG, SIGHUP);
				break;//子进程
			}
			else {//父进程
				num--;
				//printf("已成功fork个数:%d\n", connect_num - num);
			}
		}
	}

	int sockfd= initialize(1);
	if (myconnect(sockfd, ip, port, 1) < 0)
		exit(1);
	if (signal(SIGALRM, timeout) == SIG_ERR)
	{
		perror("signal");
		exit(1);
	}
	alarm(1);//设置超时时间
	if (data_trans_block(sockfd) < 0)
		exit(1);
	else
		exit(0);
}



int nonblock_fork_process(char *ip, int port, int connect_num)
{
	int num = connect_num;
	pid_t pid;
	int status = 0, ret = 0;
	int success_num = 0;
	printf("总通信client数为:%d\n", connect_num);
	while (1) {
		if (num <= 0) {//全部fork已完成
			
			ret = waitpid(-1, &status, WNOHANG);
			if (ret > 0) {//有子进程结束了
				//printf("回收进程pid:%d\n", ret);
				if (WIFEXITED(status)) {//子进程是被exit或return结束的
					if (WEXITSTATUS(status) == 1) {//exit或return值为1
						printf("end\n");
						num++;
					}
					else  if (WEXITSTATUS(status) == 0) {
						//exit或return值为0，则表示成功结束
						success_num++;
						printf("已成功通信client个数:%d\n", success_num);
						if (success_num == connect_num) {//成功通信的个数等于总个数
							return 1;
						}
					}
				}
				else {//子进程异常结束
					printf("%d进程异常退出,收到信号:%d\n", pid, status);
					num++;
				}
			}
		}
		else {
			pid = fork();//分裂子进程
			if (pid < 0) {
				perror("fork");
				continue;
			}
			if (pid == 0) {
				prctl(PR_SET_PDEATHSIG, SIGHUP);
				break;//子进程
			}
			else {//父进程
				num--;
				//printf("已成功fork个数:%d\n", connect_num - num);
			}
		}
	}

	int sockfd = initialize(0);
	if (myconnect(sockfd, ip, port, 0) < 0)
		exit(1);
	prctl(PR_SET_PDEATHSIG, SIGHUP);
	fd_set rfd, wfd;
	info cur;
	memset(&cur, 0, sizeof(cur));
	cur.fd = sockfd;
	int sign = 0;
	while (1)
	{
		FD_ZERO(&wfd);
		FD_ZERO(&rfd);
		FD_SET(cur.fd, &rfd);
		FD_SET(cur.fd, &wfd);             //Be Serious!!
		alarm(1);
		int key = select(cur.fd + 1, &rfd, NULL, NULL, NULL);
		if (key <= 0)
		{
			perror("select error");
			exit(1);
		}
		char wbuf[MAXLEN], rbuf[MAXLEN];
		int k, sj = 0;
		int rn,wlen, rlen, flag = 0;
		string ref;

		if (FD_ISSET(cur.fd, &rfd))           //read
		{
			if (cur.state > 8)
				exit(0);
			switch (cur.state)
			{
			case 0:
			{
				memset(rbuf, 0, sizeof(rbuf));
				ref = "StuNo";
				rlen = read(cur.fd, rbuf, sizeof(rbuf));
				if (rlen != ref.length())
					flag = -1;
				else
				{
					for (k = 0; k < rlen; k++)
					{
						if (rbuf[k] != ref[k])
						{
							flag = -1;
							break;
						}
					}
				}

				break;
			}
			case 2:
			{
				memset(rbuf, 0, sizeof(rbuf));
				ref = "pid";
				rlen = read(cur.fd, rbuf, sizeof(rbuf));
				if (rlen != ref.length())
					flag = -1;
				else
				{
					for (k = 0; k < rlen; k++)
					{
						if (rbuf[k] != ref[k])
						{
							flag = -1;
							break;
						}
					}
				}
				break;
			}
			case 4:
			{
				memset(rbuf, 0, sizeof(rbuf));
				ref = "TIME";
				rlen = read(cur.fd, rbuf, sizeof(rbuf));
				if (rlen != ref.length() + 1)            //尾0
					flag = -1;
				else
				{
					for (k = 0; k < rlen; k++)
					{
						if (rbuf[k] != ref[k])
						{
							flag = -1;
							break;
						}
					}
				}
				break;
			}
			case 6:
			{
				memset(rbuf, 0, sizeof(rbuf));
				ref = "str";
				rlen = read(cur.fd, rbuf, sizeof(rbuf));
				if (rlen != 9)            //尾0
					flag = -1;
				else
				{
					for (k = 0; k < 3; k++)
					{
						if (rbuf[k] != ref[k])
						{
							flag = -1;
							break;
						}
					}
					if (!flag)
					{
						int num = atoi(&rbuf[3]);
						if (num < 32768 || num>99999)
							flag = -1;
						else
							rn = num;
					}
				}
				break;
			}
			case 8:
			{
				memset(rbuf, 0, sizeof(rbuf));
				ref = "end";
				rlen = read(cur.fd, rbuf, sizeof(rbuf));
				if (rlen != ref.length())
					flag = -1;
				else
				{
					for (k = 0; k < rlen; k++)
					{
						if (rbuf[k] != ref[k])
						{
							flag = -1;
							break;
						}
					}
				}
				if (flag != -1)             //store to file
				{
					close(cur.fd);
					char filename[50];
					int mno, mpid;
					memcpy(&mno, cur.sno, sizeof(int));
					memcpy(&mpid, cur.pid, sizeof(int));
					sprintf(filename, "txt/%d.%d.pid.txt", ntohl(mno), ntohl(mpid));
					/*int file = open(filename, O_CREAT | O_TRUNC | O_WRONLY);
					write(file, cur.sno, sizeof(int));
					write(file, "\n", 1);
					write(file, cur.pid, sizeof(int));
					write(file, "\n", 1);
					write(file, cur.time, 19);
					write(file, "\n", 1);
					write(file, cur.ran, 5);
					write(file, "\n", 1);
					close(file);*/
					FILE *fp = fopen(filename, "w");
					fwrite(cur.sno, 1, 4, fp);
					fwrite("\n", 1, 1, fp);
					fwrite(cur.pid, 1, 4, fp);
					fwrite("\n", 1, 1, fp);
					fwrite(cur.time, 1, 19, fp);
					fwrite("\n", 1, 1, fp);
					fwrite(cur.ran, 1, rn, fp);
					fwrite("\n", 1, 1, fp);
					fclose(fp);

					sign = 1;
					printf("%d finished!\n", getpid());
					exit(0);
				}
				break;
			}
			default:
			{
				printf("order error!\n");
				exit(1);
			}
			}
			if (flag == -1)            //reconnect
			{
				close(cur.fd);
				memset(&cur, 0, sizeof(cur));
				exit(1);
			}
			else
			{
				cur.state++;
				if (cur.state == 9)
					exit(0);
			}
			if (1)            //确保读完写 有待商榷
			{
				switch (cur.state)
				{
				case 1:             //学号
				{
					memset(wbuf, 0, sizeof(wbuf));
					int neti = htonl(no);
					memcpy(wbuf, (char *)&neti, sizeof(int));           //Must memcpy Here 不能是strncpy
					wlen = write(cur.fd, wbuf, sizeof(int));
					if (wlen != sizeof(int))
						flag = -1;
					if (flag != -1)
						memcpy(cur.sno, wbuf, sizeof(int));
					break;
				}
				case 3:           //进程号
				{
					memset(wbuf, 0, sizeof(wbuf));
					int neti = htonl(getpid());
					memcpy(wbuf, (char *)&neti, sizeof(int));
					wlen = write(cur.fd, wbuf, sizeof(int));
					if (wlen != sizeof(int))
						flag = -1;
					if (flag != -1)
						memcpy(cur.pid, wbuf, sizeof(int));
					break;
				}
				case 5:              //时间
				{
					memset(wbuf, 0, sizeof(wbuf));
					strcpy(wbuf, (char *)format_time().data());
					wlen = write(cur.fd, wbuf, strlen(wbuf));
					if (wlen != 19)
						flag = -1;
					if (flag != -1)
						memcpy(cur.time, wbuf, strlen(wbuf));
					break;
				}
				case 7:              //随机数
				{
					srand(time(0));
					memset(wbuf, 0, sizeof(wbuf));
					for (k = 0; k < rn; k++)         //Be Serious!
					{
						wbuf[k] = rand() % 256;
					}
					wlen = write(cur.fd, wbuf, rn);
					//printf("why\n");
					if (wlen != rn)
						flag = -1;
					if (flag != -1)
						memcpy(cur.ran, wbuf, rn);
					break;
				}

				default:
				{
					printf("order error2!\n");
					exit(1);
				}

				}
				if (flag == -1)            //reconnect
				{
					close(cur.fd);
					memset(&cur, 0, sizeof(cur));
					exit(1);
				}
				else
				{
					cur.state++;
				}
			}
			else
			{
				exit(1);
			}
		}
		else
		{
		exit(1);
		}




	}


}

void add_new_client(int sockfd)
{
	int i;
	for (i = 0; i < MAX; i++)
	{
		if (!client[i].visit)
		{
			client[i].visit = 1;
			client[i].state = 0;
			client[i].fd = sockfd;
			client[i].finish = 0;
			client[i].connect = 0;
			if (sockfd > maxfd)
				maxfd = sockfd;
			return;
		}
	}
	printf("client已达上限,无法继续添加\n");
	return;
}

int nonblock_nofork_process(char *ip, int port, int connect_num)         //Control Dilemma here!
{
	int i;
	int j;
	fd_set rfd, wfd;
	int end_num = 0;
	int rest = connect_num;
	for (j = 0; j < connect_num; j++)
	{
		int sockfd = initialize(0);
		add_new_client(sockfd);
	}
	printf("socket finished!\n");
	while (1)
	{
		for (j = 0; j < MAX; j++)
		{
			if (client[j].connect == 0 && client[j].finish == 0&&client[j].visit)
			{
				printf("try to connect\n");
				if (myconnect(client[j].fd, ip, port, 0) >= 0)
				{
					printf("suc\n");
					client[j].connect = 1;
				}
				else
				{
					//shutdown(client[j].fd, 2);
					close(client[j].fd);
					client[j].fd = initialize(0);
					printf("failed\n");
				}
			}
			//printf("i am here %d\n", j);
			FD_ZERO(&wfd);
			FD_ZERO(&rfd);
			int flag = 0;
			for (i = 0; i < MAX; i++)
			{
				if (client[i].connect)
				{
					flag = 1;
					FD_SET(client[i].fd, &rfd);
					FD_SET(client[i].fd, &wfd);
				}
			}
			if (!flag)
				continue;
			struct timeval timeout = { 0,0};
			int key = select(maxfd + 1, &rfd, NULL, NULL, &timeout);
			if (key < 0)
			{
				perror("select error");
				exit(0);
			}
			else if (key == 0)
				continue;
			for (i = 0; i < MAX; i++)
			{
				char wbuf[MAXLEN], rbuf[MAXLEN];
				int k, sj = 0;
				int wlen, rlen, flag = 0;
				string ref;
				int rn;
				if (client[i].connect)
				{
					if (FD_ISSET(client[i].fd, &rfd))           //read
					{
						if (client[i].state > 8)
							continue;
						//printf("%d read %d!\n",client[i].fd,client[i].state);
						flag = 0;
						switch (client[i].state)
						{
						case 0:
						{
							memset(rbuf, 0, sizeof(rbuf));
							ref = "StuNo";
							rlen = read(client[i].fd, rbuf, sizeof(rbuf));
							if (rlen != ref.length())
								flag = -1;
							else
							{
								for (k = 0; k < rlen; k++)
								{
									if (rbuf[k] != ref[k])
									{
										flag = -1;
										break;
									}
								}
							}
							break;
						}
						case 2:
						{
							memset(rbuf, 0, sizeof(rbuf));
							ref = "pid";
							rlen = read(client[i].fd, rbuf, sizeof(rbuf));
							if (rlen != ref.length())
								flag = -1;
							else
							{
								for (k = 0; k < rlen; k++)
								{
									if (rbuf[k] != ref[k])
									{
										flag = -1;
										break;
									}
								}
							}
							break;
						}
						case 4:
						{
							memset(rbuf, 0, sizeof(rbuf));
							ref = "TIME";
							rlen = read(client[i].fd, rbuf, sizeof(rbuf));
							if (rlen != ref.length() + 1)            //尾0
								flag = -1;
							else
							{
								for (k = 0; k < rlen; k++)
								{
									if (rbuf[k] != ref[k])
									{
										flag = -1;
										break;
									}
								}
							}
							break;
						}
						case 6:
						{
							memset(rbuf, 0, sizeof(rbuf));
							ref = "str";
							rlen = read(client[i].fd, rbuf, sizeof(rbuf));
							if (rlen != 9)            //尾0
								flag = -1;
							else
							{
								for (k = 0; k < 3; k++)
								{
									if (rbuf[k] != ref[k])
									{
										flag = -1;
										break;
									}
								}
								if (!flag)
								{
									int num = atoi(&rbuf[3]);
									if (num < 32768 || num>99999)
										flag = -1;
									else
										client[i].rn = num;
								}
							}
							break;
						}
						case 8:
						{
							memset(rbuf, 0, sizeof(rbuf));
							ref = "end";
							rlen = read(client[i].fd, rbuf, sizeof(rbuf));
							if (rlen != ref.length())
								flag = -1;
							else
							{
								for (k = 0; k < rlen; k++)
								{
									if (rbuf[k] != ref[k])
									{
										flag = -1;
										break;
									}
								}
							}
							if (flag != -1)             //store to file
							{
								//close(client[i].fd);
								char filename[50];
								int mno, mpid;
								memcpy(&mno, client[i].sno, sizeof(int));
								memcpy(&mpid, client[i].pid, sizeof(int));
								sprintf(filename, "txt/%d.%d.pid.txt", ntohl(mno), ntohl(mpid));
								int file = open(filename, O_CREAT | O_TRUNC | O_WRONLY);
								write(file, client[i].sno, sizeof(int));
								write(file, "\n", 1);
								write(file, client[i].pid, sizeof(int));
								write(file, "\n", 1);
								write(file, client[i].time, 19);
								write(file, "\n", 1);
								write(file, client[i].ran, client[i].rn);
								write(file, "\n", 1);
								close(file);
								//memset(&client[i], 0, sizeof(client[i]));
								client[i].finish = 1;
								client[i].connect = 0;
								/*FILE *fp = fopen(filename, "w");
								fwrite(client[i].sno, 1, 4, fp);
								fwrite("\n", 1, 1, fp);
								fwrite(client[i].pid, 1, 4, fp);
								fwrite("\n", 1, 1, fp);
								fwrite(client[i].time, 1, 19, fp);
								fwrite("\n", 1, 1, fp);
								fwrite(client[i].ran, 1, 5, fp);
								fwrite("\n", 1, 1, fp);
								fclose(fp);*/
								end_num++;
								printf("已完成%d个 filename=%s\n", end_num, filename);
							}
							break;
						}
						default:
						{
							printf("%d order error!\n", i);
							exit(0);
						}
						}
						if (flag == -1)            //reconnect
						{
							close(client[i].fd);
							memset(&client[i], 0, sizeof(client[i]));
							client[i].visit = 1;
							client[i].fd = initialize(0);
							continue;            //!
						}
						else
						{
							client[i].state++;
							if (client[i].state == 9)
								continue;
						}
						if (1)            //确保读完写 有待商榷
						{

							switch (client[i].state)
							{
							case 1:             //学号
							{
								memset(wbuf, 0, sizeof(wbuf));
								int neti = htonl(no);
								memcpy(wbuf, (char *)&neti, sizeof(int));           //Must memcpy Here 不能是strncpy
								wlen = write(client[i].fd, wbuf, sizeof(int));
								if (wlen != sizeof(int))
									flag = -1;
								if (flag != -1)
									memcpy(client[i].sno, wbuf, sizeof(int));
								break;
							}
							case 3:           //进程号
							{
								memset(wbuf, 0, sizeof(wbuf));
								int neti = htonl((getpid() << 16) + client[i].fd);
								memcpy(wbuf, (char *)&neti, sizeof(int));
								wlen = write(client[i].fd, wbuf, sizeof(int));
								if (wlen != sizeof(int))
									flag = -1;
								if (flag != -1)
									memcpy(client[i].pid, wbuf, sizeof(int));
								break;
							}
							case 5:              //时间
							{
								memset(wbuf, 0, sizeof(wbuf));
								strcpy(wbuf, (char *)format_time().data());
								wlen = write(client[i].fd, wbuf, strlen(wbuf));
								if (wlen != 19)
									flag = -1;
								if (flag != -1)
									memcpy(client[i].time, wbuf, strlen(wbuf));
								break;
							}
							case 7:              //随机数
							{
								srand(time(0));
								memset(wbuf, 0, sizeof(wbuf));
								for (k = 0; k < client[i].rn; k++)         //Be Serious!
								{
									wbuf[k] = rand() % 256;
								}
								wlen = write(client[i].fd, wbuf, client[i].rn);
								//printf("why\n");
								if (wlen != rn)
									flag = -1;
								if (flag != -1)
									memcpy(client[i].ran, wbuf, client[i].rn);
								break;
							}

							default:
							{
								printf("%d order error2!\n", i);
								exit(0);
							}

							}
							if (flag == -1)            //reconnect
							{
								close(client[i].fd);
								memset(&client[i], 0, sizeof(client[i]));
								client[i].fd = initialize(0);
								client[i].visit = 1;
								continue;
							}
							else
							{
								client[i].state++;
							}
						}
					}
				}

			}
			if (end_num == connect_num)             //退出
			{
				printf("last round finished! end_num=%d\n", end_num);
				printf("All Finished!\n");
				exit(0);
			}

		}
		printf("round finished! end_num=%d\n", end_num);
	}
	for (j = 0; j < MAX; j++)
	{
		if (client[j].visit)
			close(client[j].fd);
	}
	return 1;
	
}



int main(int argc, char **argv)
{
	system("mkdir txt");
	//system("rm -f ./txt/*.txt");
	char ip[50];
	int port, isblock, isfork, connect_num;
	if (get_parameter(argc, argv, ip, &port, &isblock, &isfork, &connect_num) < 0)
	{
		printf("Parameter Error!\n");
		return 0;
	}
	string tol[2] = { "no","yes" };
	printf("Parameter Info:\nip:%s\nport:%d\nconnect_num:%d\n", ip, port, connect_num);
	cout << "block:" << tol[isblock] << endl << "fork:" << tol[isfork] << endl;
	//signal(SIGCHLD, myjudge);
	if (isblock)
	{
		block_process(ip, port, connect_num);
	}
	else if (isfork)
	{
		nonblock_fork_process(ip, port, connect_num);
	}
	else
		nonblock_nofork_process(ip, port, connect_num);
	printf("FINISHED!\n");
	return 0;
}




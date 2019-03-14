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

#define MAX_LEN 1600
#define IP_ADDR_LEN 4

typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

typedef struct _eth_hdr
{
	unsigned char dstmac[6];
	unsigned char srcmac[6];
	unsigned short type;
}eth_hdr;

typedef struct ip_hdr
{
	unsigned char len_ver;			//首部长度+版本
	unsigned char tos;				//服务类型
	unsigned short iplen;			//总长
	unsigned short identification;	//标志
	//unsigned short offset:13;		//偏移
	unsigned short flag:3;			//flag
	unsigned short offset:13;		//偏移
	unsigned char ttl;				//生存时间
	unsigned char proto;			//协议
	unsigned short cksum;			//检验和
	unsigned int srcip;			//源IP地址
	unsigned int dstip;			//目的IP地址
}ip_hdr;

typedef struct ip_hdr_net
{
	unsigned char len_ver;			//首部长度+版本
	unsigned char tos;				//服务类型
	unsigned short iplen;			//总长
	unsigned short identification;	//标志
	//unsigned short flag:3;			//flag
	//unsigned short offset:13;		//偏移
	unsigned short offset_flag;
	unsigned char ttl;				//生存时间
	unsigned char proto;			//协议
	unsigned short cksum;			//检验和
	unsigned int srcip;			//源IP地址
	unsigned int dstip;			//目的IP地址
}ip_hdr_net;

typedef struct _tcp_hdr
{
	unsigned short srcport;
	unsigned short dstport;
	unsigned int seq;
	unsigned int ack;
	unsigned char offset : 4;
	unsigned char reserved : 6;
	unsigned char flag : 6;
	unsigned short window;
	unsigned short cksum;
	unsigned short urg;
	//extern
}tcp_hdr;

typedef struct _tcp_hdr_net
{
	unsigned short srcport;
	unsigned short dstport;
	unsigned int seq;
	unsigned int ack;
	//unsigned char offset : 4;
	//unsigned char reserved : 6;
	//unsigned char flag : 6;
	unsigned short orf;
	unsigned short window;
	unsigned short cksum;
	unsigned short urg;
	//extern
}tcp_hdr_net;



uint16_t ip_chksum(uint16_t initcksum, uint8_t *ptr, int len)
{
    unsigned int cksum;
    int idx;
    int odd;

    cksum = (unsigned int) initcksum;

    odd = len & 1;
    len -= odd;

    for (idx = 0; idx < len; idx += 2) {
        cksum += ((unsigned long) ptr[idx] << 8) + ((unsigned long) ptr[idx+1]);
    }

    if (odd) {      /* buffer is odd length */
        cksum += ((unsigned long) ptr[idx] << 8);
    }

    /*
     * Fold in the carries
     */

    while (cksum >> 16) {
        cksum = (cksum & 0xFFFF) + (cksum >> 16);
    }

    return cksum;
}

unsigned short checksum(unsigned short *buf, int size) {
	unsigned long cksum = 0;
	while (size > 1) {
		cksum += *buf++;
		size -= sizeof(unsigned short);
	}
	if (size) {
		cksum += *(unsigned char*)buf;
	}

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (unsigned short)(~cksum);
}

uint16_t tcp_chksum(uint8_t *tcphead, int tcplen , uint32_t *srcaddr, uint32_t *destaddr)
{
    uint8_t pseudoheader[12];
    uint16_t calccksum;

    memcpy(&pseudoheader[0],srcaddr,IP_ADDR_LEN);
    memcpy(&pseudoheader[4],destaddr,IP_ADDR_LEN);
    pseudoheader[8] = 0; /* 填充0*/
    pseudoheader[9] = IPPROTO_TCP;
    pseudoheader[10] = (tcplen >> 8) & 0xFF;
    pseudoheader[11] = (tcplen & 0xFF);

	unsigned short buff[1600];
	unsigned char sendf[1600];
	memcpy(sendf,&pseudoheader,12);//此处有坑:先char拷贝合并，再拷贝进short
	memcpy(sendf+12,tcphead,tcplen);

	memcpy(buff,sendf,tcplen+12);
	return checksum(buff,tcplen+sizeof(pseudoheader));
}



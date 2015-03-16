#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <error.h>
using namespace std;


#define PACKET_SIZE 4096
#define DATA_LENGTH 56

int send_num = 0;
int recv_num = 0;
struct sockaddr_in dstaddr;
struct sockaddr_in from;
pid_t  pid;
int icmpfd;
struct timeval tvrecv;


void tv_sub(struct timeval *out,struct timeval *in)
{       
	if( (out->tv_usec-=in->tv_usec)<0)
        	{       	--out->tv_sec;
                	out->tv_usec+=1000000;
        	}
        	out->tv_sec-=in->tv_sec;
}

unsigned short cal_chksum(unsigned short *addr,int len)
{
	int nleft = len;
    	int sum = 0;
    	unsigned short *w = addr;
    	unsigned short check_sum = 0;

    	while(nleft>1)      //ICMP包头以字（2字节）为单位累加
    	{
	        sum += *w++;
	        nleft -= 2;
   	}

    	if(nleft == 1)      //ICMP为奇数字节时，转换最后一个字节，继续累加
    	{
        		*(unsigned char *)(&check_sum) = *(unsigned char *)w;
        		sum += check_sum;
    	}
    	sum = (sum >> 16) + (sum & 0xFFFF);
    	sum += (sum >> 16);
    	check_sum = ~sum;   //取反得到校验和
    	return check_sum;
}

//构建ICMP数据包
int pack(int pack_no,char *buf)
{
	int packsize;
	struct timeval *tval;
	struct icmp *icmp = (struct icmp*)buf;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_seq = pack_no;
	icmp->icmp_id = pid;

	packsize = 8+DATA_LENGTH;
	tval = (struct timeval*)icmp->icmp_data;
	gettimeofday(tval, NULL);
	icmp->icmp_cksum = cal_chksum((unsigned short *)icmp, packsize);
	return packsize;
}

//分析收到的icmp反馈的结果
int unpack(char *buf,int len)
{
	int i,iphdrlen;
        	struct ip *ip;
        	struct icmp *icmp;
        	struct timeval *tvsend;
        	double rtt;

        	ip=(struct ip *)buf;
        	iphdrlen=ip->ip_hl<<2;    /*求ip报头长度,即ip报头的长度标志乘4*/
        	icmp=(struct icmp *)(buf+iphdrlen);  /*越过ip报头,指向ICMP报头*/
        	len-=iphdrlen;            /*ICMP报头及ICMP数据报的总长度*/
        	if( len<8)                /*小于ICMP报头长度则不合理*/
        	{       
        		cout<<"ICMP packet size less than 8 bytes"<<endl;
                	return -1;
        	}
        	/*确保所接收的是我所发的的ICMP的回应*/
        	if( (icmp->icmp_type==ICMP_ECHOREPLY) && (icmp->icmp_id==pid) )
        	{       
        		tvsend=(struct timeval *)icmp->icmp_data;
                	tv_sub(&tvrecv,tvsend);  /*接收和发送的时间差*/
                	rtt=tvrecv.tv_sec*1000+tvrecv.tv_usec/1000;  /*以毫秒为单位计算rtt*/
        		printf("%d bytes icmp_seq=%u ttl=%d time=%.1f ms\n",len,  icmp->icmp_seq,ip->ip_ttl,rtt);
        	}else
        		return -1;
}

//发送icmp数据包
void *send_packet(void *arg)
{
	int packet_size;
	char sendPacket[PACKET_SIZE];
	while(1)
	{
		memset(sendPacket, 0, PACKET_SIZE);
		packet_size = pack(send_num, sendPacket);
		if(sendto(icmpfd,sendPacket,packet_size,0,(struct sockaddr*)&dstaddr,sizeof(dstaddr)) < 0)
		{
			cout<<"socket sendto error"<<endl;
		}
		send_num++;
		sleep(1);
	}
}

//接收icmp数据包
void *recv_packet(void *arg)
{
	int rv,n;
	char recvPacket[PACKET_SIZE];
	fd_set readset,testset;
	FD_ZERO(&readset);
	FD_SET(icmpfd, &readset);
	memset(&from, 0, sizeof(from));
	socklen_t formlen = sizeof(from);
	while(1)
	{
		testset = readset;
		rv = select(icmpfd+1,&testset,NULL,NULL,0);
		if(rv == 0)
		{
			cout<<"ping timeout"<<endl;
		}else if(rv > 0){
			n = recvfrom(icmpfd,recvPacket,sizeof(recvPacket),0,(struct sockaddr*)&from,&formlen);
			gettimeofday(&tvrecv, NULL);
			if(n > 0)
			{
				unpack(recvPacket, n);
				recv_num++;
			}else{
				cout<<"receive error"<<endl;
			}
		}else{
			cout<<"select error"<<endl;
		}

	}
}
int main(int argc, char const *argv[])
{
	struct ifreq ifr;
	pthread_t send_thread,recv_thread;
	if((icmpfd = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP)) < 0)
	{
		perror("socket");
		return -1;
	}
	memset(&dstaddr, 0, sizeof(dstaddr));
	dstaddr.sin_family = AF_INET;
	dstaddr.sin_addr.s_addr = inet_addr("222.128.13.159");
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	setsockopt(icmpfd,SOL_SOCKET,SO_BINDTODEVICE,(char*)&ifr,sizeof(ifr));
	pid = getpid();
	pthread_create(&send_thread, 0, send_packet, NULL);
	pthread_create(&recv_thread, 0, recv_packet, NULL);
	pthread_join(send_thread, 0);
	pthread_join(recv_thread, 0);
	return 0;
}
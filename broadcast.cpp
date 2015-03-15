// 发送端
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>


using namespace std;

void *send_thread(void *arg)
{
	int sock = *(int*)arg;
	struct sockaddr_in addrto;
	bzero(&addrto, sizeof(struct sockaddr_in));
	addrto.sin_family=AF_INET;
	addrto.sin_addr.s_addr=inet_addr("10.103.255.255");
	//addrto.sin_addr.s_addr=htonl(INADDR_BROADCAST);
	addrto.sin_port=htons(6000);
	int nlen=sizeof(addrto);

	while(1)
	{
		sleep(1);
		//从广播地址发送消息
		char smsg[] = {"abcdef"};
		int ret=sendto(sock, smsg, strlen(smsg), 0, (sockaddr*)&addrto, nlen);
		if(ret<0)
		{
			cout<<"send error...."<<ret<<endl;
		}
	}
}

int main()
{
	setvbuf(stdout, NULL, _IONBF, 0); 
	fflush(stdout); 

	int sendfd = -1;
	pthread_t send_t,recv_t;

	if (( sendfd= socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
	{   
		cout<<"socket error"<<endl;	
		return false;
	}   
	
	const int opt = 1;
	//设置该套接字为广播类型，
	int nb = 0;
	nb = setsockopt(sendfd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
	if(nb == -1)
	{
		cout<<"set socket error..."<<endl;
		return false;
	}

	pthread_create(&send_t, 0, send_thread, &sendfd);

	// 绑定地址
	struct sockaddr_in addrto;
	bzero(&addrto, sizeof(struct sockaddr_in));
	addrto.sin_family = AF_INET;
	addrto.sin_addr.s_addr = htonl(INADDR_ANY);
	addrto.sin_port = htons(6000);

	struct sockaddr_in from;
	memset(&from, 0, sizeof(from));
	
	int recvfd = -1;
	if ((recvfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		cout<<"socket error"<<endl;	
		return false;
	}

	if(bind(recvfd,(struct sockaddr *)&(addrto), sizeof(struct sockaddr_in)) == -1) 
	{   
		cout<<"bind error..."<<endl;
		return false;
	}

	int len = sizeof(sockaddr_in);
	char smsg[100] = {0};

	while(1)
	{
		//从广播地址接受消息
		int ret=recvfrom(recvfd, smsg, 100, 0, (struct sockaddr*)&from,(socklen_t*)&len);
		if(ret<=0)
		{
			cout<<"read error...."<<ret<<endl;
		}
		else
		{		
			printf("%s\t", smsg);	
		}
		sleep(1);
	}

	return 0;
}

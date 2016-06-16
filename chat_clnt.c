#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20
#define GROUP_SIZE 20

#pragma pack(push, 1)
typedef struct Packet {
	int join_flag;
	int sock;
	char name[NAME_SIZE];
	char group[GROUP_SIZE];
	char msg[BUF_SIZE];
} Packet;
#pragma pack(pop)


void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);
	
	
int main(int argc, char *argv[])
{
	Packet packet;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	if(argc!=5) {
		printf("Usage : %s <IP> <port> <name> <group>\n", argv[0]);
		exit(1);
	 }
	packet.join_flag = 0;
	sprintf(packet.name, "%s", argv[3]);
	sprintf(packet.group, "%s", argv[4]);
	packet.sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(packet.sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
	printf("Welcome to chatting Server.\n");
	printf("[gourp]\t\tmessage\t\t(clinet)\n");
	send(packet.sock, (char*)&packet, sizeof(Packet), 0);


	pthread_create(&snd_thread, NULL, send_msg, &packet);
	pthread_create(&rcv_thread, NULL, recv_msg, &packet.sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(packet.sock);  
	return 0;
}
	
void * send_msg(void * arg)   // send thread main
{
	Packet sendPacket = *((Packet*) arg);
	sendPacket.join_flag = 1;
	while(1) 
	{
		fgets(sendPacket.msg, BUF_SIZE, stdin);
		sendPacket.msg[(int)strlen(sendPacket.msg)-1] = '\0';
		if(!strcmp(sendPacket.msg,"q\n")||!strcmp(sendPacket.msg,"Q\n")) 
		{
			close(sendPacket.sock);
			exit(0);
		}
		send(sendPacket.sock, (char*)&sendPacket, sizeof(sendPacket), 0);
	}
	return NULL;
}
	
void* recv_msg(void* arg)   // read thread main
{
	int sock = *((int*) arg);
	int packet_len;
	char buf[sizeof(Packet)];
	Packet* recvPacket;
	while((packet_len=recv(sock, buf, sizeof(Packet), 0)) != 0)
	{
		buf[packet_len] = '\0';
		recvPacket = (Packet*) buf;
		if(recvPacket->join_flag == 0)
		{
			printf("It took part in the '%s' chat room '%s'.\n", recvPacket->name, recvPacket->group);
		}
		else
		{
			printf("[Room %s]\t\t%s  (%s)\n", recvPacket->group, recvPacket->msg, recvPacket->name);
		}
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20
#define GROUP_SIZE 20
#define MAX_CLNT 256

#pragma pack(push, 1)
typedef struct Packet {
	int join_flag;
	int sock;
	char name[NAME_SIZE];
	char group[GROUP_SIZE];
	char msg[BUF_SIZE];
} Packet;
#pragma pack(pop)

typedef struct Client {
	int clnt_sock;
	char group[GROUP_SIZE];
} Client;

void* handle_clnt(void* arg);
void send_msg(int clnt_sock, Packet sendPacket,  int length);
void error_handling(char * msg);

int clnt_cnt=0;
Client client[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	int optval = 1;
	pthread_t t_id;


	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		client[clnt_cnt++].clnt_sock = clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
	}
	close(serv_sock);
	return 0;
}
	
void* handle_clnt(void* arg)
{
	int clnt_sock = *((int*)arg);
	Packet* recvPacket;
	Packet sendPacket;
	char buf[sizeof(Packet)];
	int packet_len=0, i;
	
	while((packet_len=recv(clnt_sock, buf, sizeof(Packet),0)) != 0)
	{
		buf[packet_len] = '\0';
		recvPacket = (Packet*) buf;
		if(recvPacket->join_flag == 0)
		{
			printf("'%s' is to attend to the chat room '%s'\n", recvPacket->name, recvPacket->group);
			pthread_mutex_lock(&mutx);
			for(i = 0; i < clnt_cnt; i++)
			{
				if(client[i].clnt_sock == clnt_sock)
				{
					sprintf(client[i].group, "%s", recvPacket->group);
					break;
				}
			}
			pthread_mutex_unlock(&mutx);
		}
		else
		{
			printf("'%s' message in a chat room '%s': %s\n", recvPacket->name, recvPacket->group, recvPacket->msg);
			sprintf(sendPacket.msg, "%s", recvPacket->msg);
		}
		sendPacket.join_flag = recvPacket->join_flag;
		sprintf(sendPacket.name, "%s", recvPacket->name);
		sprintf(sendPacket.group, "%s", recvPacket->group);
		send_msg(clnt_sock, sendPacket, sizeof(Packet));
	}
	
	pthread_mutex_lock(&mutx);
	for(i=0; i < clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==client[i].clnt_sock)
		{
			while(i++<clnt_cnt-1)
				client[i]=client[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

void send_msg(int clnt_sock, Packet sendPacket,  int length)
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i < clnt_cnt; i++)
	{
		if(client[i].clnt_sock != clnt_sock && strcmp(client[i].group, sendPacket.group) == 0)
		{
			send(client[i].clnt_sock, (char*)&sendPacket, length, 0);
		}
	}
	pthread_mutex_unlock(&mutx);

}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

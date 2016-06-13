#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "/usr/include/mysql/mysql.h"
#include "packet.h"
#define BUF_SIZE 1024
#define SELECT_RECODE "select * from sign_table"
void* client_connection(void* arg);
void log_write(char* message);
void send_meesage(int client, Packet packet, int length);

pthread_mutex_t mutex;
MYSQL mysql;
MYSQL_RES *result;
MYSQL_ROW row;
int query_state;
char error_log[1024];

int main(int argc, char* argv[])
{
	int server_socket, client_socket;
	struct sockaddr_in server_address, client_address;
	char error[BUF_SIZE];
	int optval = 1;
	socklen_t client_address_size;
	pthread_t thread[2];
	
	if(argc != 2)
	{
		printf("Usage : %s <port>\n",argv[0]);
		sprintf(error, "Usage : %s <port>", argv[0]);
		log_write(error);
		exit(1);
	}


	if(pthread_mutex_init(&mutex, NULL))
	{
		log_write("pthread_mutex_init() error");
		exit(1);
	}

	if( (server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		log_write("socket() error.");
		exit(1);
	}
	
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	memset(&server_address, 0 , sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[1]));
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
	{
		log_write("bind() error.");
		exit(1);
	}

	if(listen(server_socket, 5) == -1)
	{
		log_write("listen error.");
		exit(1);
	}
	
	printf("running server.\n\n");
	
	// DB 연결
	mysql_init(&mysql);
	if( mysql_real_connect(&mysql, "localhost", "root", "1234", "sign_db", 3306, NULL, 0) == NULL)
	{
		log_write("mysql_real_connect() error");
		exit(1);
	}
	printf("using 'sgin_db'.\n");

	
	while(1)
	{
		client_address_size = sizeof(client_address);

		if((client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size)) == -1)
		{
			log_write("accept() error.");
			exit(1);
		}
		
		pthread_create(&thread[0], NULL, client_connection, (void*)&client_socket);
	//	pthread_create(&thread[1], NULL, recode_update, NULL);
	}		
	
	mysql_close(&mysql);
	close(server_socket);
	exit(0);
}

void* client_connection(void* arg)
{
	int client_socket = *((int*) arg);
	int pack_len = 0;
	int count = 0;
	int total_recode;
	Packet* recvPacket;
	Packet sendPacket;
	char buffer[sizeof(Packet)];

	char query[100];
	while((pack_len = read(client_socket, buffer, sizeof(buffer))) != 0 )
	{
		buffer[pack_len] = '\0';
		recvPacket = (Packet*)buffer;
		if(recvPacket->ack_flag == 1)
		{
			printf("Client: ACK\n");
		if((query_state = mysql_query(&mysql, SELECT_RECODE)) != 0)
		{
			sprintf(error_log, "%s", mysql_error(&mysql));
			printf("%s\n", error_log);
			log_write(error_log);
			exit(1);
		}
		result = mysql_store_result(&mysql);
		total_recode = mysql_num_rows(result);
		if(total_recode == 0)
		{
			printf("No data on the table.\n");
			sendPacket.ack_flag = 1;
			sendPacket.isExist = 0;
			count = 0;
		}
		else
		{
			printf("toal_recode : %d/ count: %d\n", total_recode, count);
			sprintf(query, "select * from sign_table order by 'regi_day' desc limit %d, 1", count);
			if((query_state = mysql_query(&mysql, query)) != 0)
			{
				sprintf(error_log,"%s", mysql_error(&mysql));
				log_write(error_log);
				exit(1);
			}
			result = mysql_store_result(&mysql);
			if((row = mysql_fetch_row(result)) != NULL)
			{
				sendPacket.ack_flag = 1;
				sendPacket.isExist = 1;
				sprintf(sendPacket.message, "%s", row[0]);
				printf("sendPacket.message: %s\n", sendPacket.message);
			}
			if(count < total_recode-1)
			{
				count++;
			}
			else
			{
				count = 0;
			}
			printf("DB data: %s\n", row[0]);
		}
		
		mysql_free_result(result);
		send_meesage(client_socket, sendPacket, sizeof(Packet));
		sleep(3);
		//strcpy(buffer, "");
		}
	}
	close(client_socket);
}



void send_meesage(int client_socket, Packet packet, int length)
{
	
	pthread_mutex_lock(&mutex);
	write(client_socket, (char*)&packet, length);
	pthread_mutex_unlock(&mutex);
}


void log_write(char* message)
{
	FILE *logFile = fopen("log.txt", "a");
	struct tm *t;
	time_t timer;
	
	timer = time(NULL);
	t = localtime(&timer);
	printf("%s\n", message);
	fprintf(logFile, "%04d-%02d-%02d %02d:%02d:%02d %s\n",
			t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec,
			message);
	fclose(logFile);
}


#include <stdio.h>	// using fputs(), fputc()
#include <stdlib.h>	// using exit()
#include <unistd.h>	// using close() 
#include <string.h>	// using memset()

// using socket(), bind(), listen(), accept(), htons(), htonl(), inet_ntoa()
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//using pthread_mutex_lock(), pthread_mutex_unlock(), pthread_create()
#include <pthread.h>

#define BUFFER_SIZE 1024

// prototype functions
void* client_connection(void *arg);
void send_message(int client_socket, char* message, int length);
void error(char *message);

typedef struct USER{
	char name[11];
} USER;

//global value
int client_number = 0;
int client_socket_array[10];
USER user[10];
pthread_mutex_t mutex;

int main()
{
	// local value
	int server_socket;	// ���� ���� ���� ��ũ����
	int client_socket;	// Ŭ���̾�Ʈ ���� ���� ��ũ����
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int client_address_size;	// Ŭ���̾�Ʈ �ּ� ũ��
	pthread_t thread;
	int optval = 1; // ���� ���� ����� ��õ� ������ �ش� ��Ʈ�� ���ε��� �ٽ� �� �� �ִ�.
	int i;
					
	// ���� ó���� ����� ���� �⺻ ����
	// ���ؽ� �ʱ�ȭ ����ó��
	if(pthread_mutex_init(&mutex, NULL))
	{
		error("Muexex Init Error!");
	}

	//���� ������ IPv4 ���ͳ� �������� ü��� ����Ѵ�.
	//���� �ؽ�Ʈ �޽����� �����Ƿ� TCP/IP ���������� ����Ѵ�.
	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	
	// ���� ������ ���� ���� ó��
	if (server_socket == -1)
	{
		error("Socket() Error!");
	}

	// ������ �簡���� ���������� ���� ��Ʈ�� ���Ӱ� ���ε��� �� �� �ֵ��� �����Ѵ�.
	setsockopt(server_socket,SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	
	// ���� ����ü�� ������ 0 �Ǵ� NULL ������ �ʱ�ȭ
	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;	//���� �ּҴ� IPv4 ���ͳ� �������� �ּ� �ǰ��� �����Ѵ�.
	server_address.sin_port = htons(3333);	//������ ��Ʈ �ּҸ� �����Ѵ�.
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);	// ���� �ý����� ���� �ּҸ� ã�� ���� �Ѵ�.

	// ���� ������ ���� ���� ó��
	if(bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1)
	{
		error("Bind() Error!");
	}

	if(listen(server_socket, 5) == -1)
	{
		error("Listen() Error!");
	}

	printf("Welcome to Chatting Server.\n");
	printf("LISTENING...\n");
	/*****************************/

	// ���������� �������� Ŭ���̾�Ʈ�� ��û�� ó��
	while(1)
	{
		client_address_size = sizeof(client_address);	//Ŭ���̾�Ʈ�� �ּ� ũ���� 4Byte�� ���� 

		// Ŭ���̾�Ʈ�� ���� ��û�� �����ϰ�
		// �ش� Ŭ���̾�Ʈ�� ���� ��ũ��Ʈ ��ȣ�� �������ش�.
		client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_size);

		// Ŭ���̾�Ʈ�� ���� ��
		if (client_socket == -1)
		{
			error("Accept() Fail");
		}

		// Ŭ���ξ�Ʈ�� ���� ��ũ���͸� ����Ѵ�.
		pthread_mutex_lock(&mutex);	// �� �۾��߿��� �ٸ� ������� �۾��� �� �� ����.
		client_socket_array[client_number++] = client_socket;	// Ŭ���̾�Ʈ�� ������ ����� �ϰ� Ŭ���̾�Ʈ ������ ���� ���� ��Ų��.
//		printf("���� ������ %d��\n",client_number);	// ���� ������ ���� ����Ѵ�.
		pthread_mutex_unlock(&mutex);	// ���� �ٸ� ��������� �ٸ� �۾��� �� �� �ְ� �Ѵ�.

		// thread�� �����Ѵ�.(�ɼ� : �⺻��)
		// �����忡 ������ �Լ��� client_connection�̸�,
		// Ŭ���̾�Ʈ ������ ���ڰ����� �Ѱ��ش�.
		// intptr_t�� int�� ������ Ÿ���̸�,
		// ������ �ܰ迡�� ������ Ÿ�Կ� �����ֱ� ���� intptr_t�� ĳ���ÿ�����
		// void* ������ �ٽ� ĳ���ÿ����� �����Ͽ� �Ѱ���.
		pthread_create(&thread, NULL, client_connection, (void *)(intptr_t)client_socket);

	}
	/***************************/

	exit(EXIT_SUCCESS);
}

// Ŭ���̾�Ʈ ���� function
void* client_connection(void *arg)
{
	int client_socket = (intptr_t) arg;	// ���� ���� �Ѱ��ش�.
	int string_length = 0;	// ���ڿ� ���� Ȯ��
	char message[BUFFER_SIZE];	// ���� �޽���
	char init_message[BUFFER_SIZE];
	int needless_string_length = 0;
	int i, j;	// �ݺ���

	// �޽ý� ����
	while((string_length = read(client_socket, message, sizeof(message))) != 0)
	{
		/*
		   ����ڰ� ä�ù濡 ���� �������� ���� �� ä���ϴ� ��츦 �����
		   Ȯ�������� Ŭ���̾�Ʈ�� ä�ø޽����� �Է� �ϴ� ��찡 �����Ƿ� 
		   ������ �켱������ ù��° ���ڰ� '['�� Ȯ�� �ϴ� ���� ���� �ӵ���
		   ���� �� ���� �� �� ���� ���̶� �����Ͽ� ������ ���� �ۼ� �Ͽ���.
		 */

		
		if(message[0] == '[')	// ä�� �޽����� �Է� ���� ���
		{
			/*
			   ������ Ŭ���̾�Ʈ�� ���� �޽����� ����ڸ� �Է��ߴ��� �˾��ϹǷ�
			   �����ڰ� ������ ������ ��Ŀ� ���� Ŭ���̾�Ʈ�� �Է��� ������ �Ľ��Ѵ�.
			*/

			// ������ ����� �ϴ� ������ �д´�.
			for (i = 0; message[i] != '\0'; i++)
			{
				/*
				   �� �۾��� ���� Ȯ�������� ������ �Ͽ� �ۼ��Ͽ���.
				*/

				if(message[i] == '[') // ������ ����� ù��° �����ΰܿ�
				{
					
					needless_string_length++; // ���ʿ��� ���� ���� 1���� ��Ų��.
					continue;	// �ؿ� ������ �����ϱ� ���� ��� �Ͽ���.
				}
				else if(message[i] != ']')	// ����� �̸��� ���
				{
					printf("%c",message[i]);	// ����� �̸��� ���ڸ� �ϳ��� ����Ѵ�.
					needless_string_length++;	// 1����
					continue;	// �Ʒ� ���ǹ��� ���� �ʱ� ���� ó��
				}
				else	// ������� �̸��� ������ �˼��ִ� ']'���� �ΰ��
				{
					needless_string_length += 4;	// ' ', ':', ' ' �� �ֱ� ������ �� ���ڵ� ���� �����ش�.
					printf(" has enterd %d words\n", (int) strlen(message) - needless_string_length);	// �� �޽��� ���� �ۼ��Ѵ�.
					needless_string_length = 0;	// ���ο� �޽����� ���� �ʱ�ȭ ���ش�.
					break;	//���ϴ� ����� ������Ƿ� �ݺ����� �������´�.
				}		
			}
		}
		else	// Ŭ���̾�Ʈ�� ä�ù濡 ������ ��� ó���Ѵ�.
		{
			strcpy(user[client_number-1].name, message);	// �ش� Ŭ���̾�Ʈ�� �̸��� Ŭ���̾�Ʈ ���� �迭�� �Ȱ��� �ε����� �����Ѵ�.
			printf("Connected %s\n", message);	// ���� �����ߴ��� ������ �˷��ش�.
			sprintf(init_message, "Connected %s", message);	// �� ������ �ٸ� Ŭ���̾�Ʈ���� ������ش�.
			strcpy(message, init_message);
		}
		
		// �ٸ� Ŭ���̾�Ʈ���� ���� �޽����� �����ش�.
		send_message(client_socket, message, string_length);
	}
	/*******************/
	
	/*
	   �� �κ��� Ŭ���̾�Ʈ�� ������ ���� �Ǹ�
	   ������ �ٸ� Ŭ���̾�Ʈ���� ���� ���� �Ǿ����� �˷��ִ� �����̴�.
	*/

	pthread_mutex_lock(&mutex);	// �� �۾� �߿��� �ٸ� ������� ����� �� ����.
	for(i = 0; i < client_number; i++)	// Ŭ���̾�Ʈ �� ��ŭ �ݺ��Ѵ�.
	{
		if(client_socket == client_socket_array[i])	// ������ Ȯ���� Ŭ���̾�Ʈ�� ����� Ŭ���̾�Ʈ�� ���
		{
			// ������ ��� Ŭ���̾�Ʈ�� ������ �������� ����Ѵ�.
			printf("%s ���� ����\n", user[i].name);	

			// ������ �ٸ� Ŭ���̾�Ʈ���� ��µ� ������ �Ȱ��� ���� �غ� �Ѵ�.
			sprintf(message, "%s ���� ����", user[i].name);

			// Ŭ���̾�Ʈ �� ��ŭ �ݺ��Ѵ�.
			for(j = 0; j <client_number; j++)
			{
				// ������ ���� Ŭ���̾�Ʈ�� ������
				// �ٸ� Ŭ���̾�Ʈ���� ������ ��µ� �޽�����  ������.
				if(client_socket_array[j] != client_socket)
				{
					write(client_socket_array[j], message, sizeof(message));
				}
			}
			
			// ������ ������ ���� Ŭ���̾�Ʈ�� ���� ��Ű�� �۾��� �Ѵ�.
			for(; i < client_number - 1; i++)
			{
				client_socket_array[i] = client_socket_array[i+1];
				user[i] = user[i+1];
			}
			break;	// �ش� �۾��� ��� ���� �����Ƿ� �ݺ����� ���� ���´�.
		}
	}// end for()

	client_number--;	// Ŭ���̾�Ʈ�� ���� 1���� ��Ų��.
//	printf("���� ������ %d��\n",client_number); // ���� ������ ���� �ٽ� Ȯ���Ѵ�.
	pthread_mutex_unlock(&mutex);	// �ٸ� �����尡 �۾��� �� �� �ְ� ���ش�.
	/********************/

	// ���� ����
	close(client_socket);

	return 0;
}

// �޽��� �۽�
void send_message(int client_socket, char* message, int length)
{
	int i; // �ݺ���
	
	// �� �۾��� ���� �߿��� �ٸ� �����忡�� �۾��� �� �� ����.
	pthread_mutex_lock(&mutex);
	for(i = 0; i < client_number; i++)
	{
		if(client_socket_array[i] != client_socket)
		{
			write(client_socket_array[i], message, length);
		}
	}
	pthread_mutex_unlock(&mutex);
}

// ���� �޽��� ���
void error(char *message)
{
	fputs(message, stderr); // ǥ�ؿ����� �ش� �޽����� ����Ѵ�.
	fputc('\n', stderr);	// ���๮�� �߰��Ѵ�.
	exit(EXIT_FAILURE);		// ������ ������ �����ߴٰ� OS�� �˷� �ش�.
}


#include <stdio.h>  // using fputs(), fputc()
#include <stdlib.h> // using exit()
#include <unistd.h> // using close() 
#include <string.h> // using memset()
#include <ctype.h>	// using isalnum() -> �ش� ���ڰ� ���� �������� ���� �������� �ǵ��ϴ� �Լ�
#include <signal.h> // using kiil()

// using socket(), bind(), listen(), accept(), htons(), htonl(), inet_ntoa()
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
 

#define BUFFER_SIZE 1024

// ����ü ����
typedef struct SOCKET_INFO{
	int server_socket;
	char name[11];
} SOCKET_INFO;

// prototype function
void send_message(SOCKET_INFO soket_info);
void receive_message(SOCKET_INFO soket_info);
void error(char *message);

int main()
{
	SOCKET_INFO format;	//������ ������ ���� ����
	struct sockaddr_in server_address;	// ������ ���� ���� ����
	char init_message[BUFFER_SIZE];
	pid_t pid; // �θ�� ������ �ڽ��� �۽��� ���ش�.
	int i;
	int name_flag;	// ����� �̸��� ��Ģ�� �´��� �ǵ� �ϴ� flag

	// Ŭ���̾�Ʈ ����� ����
	// ����� ���� �ݵ�� ��� �����̾�� �Ѵ�.
	while(1)
	{
		printf("What your name? -> ");
		fgets(format.name, 11, stdin);
		format.name[strlen(format.name)-1] = '\0';	// fgets���� �Էµ� ���๮�ڸ� null ���ڷ� ����
		
		// ��Ģ�� �°� �Է��ߴ��� Ȯ��
		for(i = 0; format.name[i] != '\0'; i++)
		{
			if(isalnum(format.name[i]))	// ���ڸ� �ϳ��� �ǵ��Ѵ�.
			{
				name_flag = 0;	// ����� �̸��� Ư�� ���ڰ� ����.
			}
			else
			{
				name_flag = 1;	// ����� �̸��� Ư�����ڸ� �߰��ߴ�.
			}
		}
		
		if(name_flag)	// �̸��� �̻��� �ִ°�?
		{
			printf("Try Agine...\n");	// ���Է� �õ�
		}
		else
		{
			break;	// ���� �۾� ����
		}
	}
	sprintf(init_message, "%s", format.name);	// �������� Ŭ���̾�Ʈ�� �̸��� ������.
	
	// ������ IPv4 ���ͳ� �������� ü��� ����Ѵ�.
	// ���� �ؽ�Ʈ �޽����� ������ �ǹǷ� TCP/IP ����������
	// ��� �ؾ� �ǹǷ� SOCK_STREAM�� ����Ѵ�.
	format.server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if(format.server_socket == -1)
	{
		error("Socket() Error!");
	}
	
	// server_address ����ü�� 0 �Ǵ� NULL ������ �ʱ�ȭ
	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;	// IPv4 ���ͳ� �������� �ּ� ü��� �����Ѵ�.
	server_address.sin_port = htons(3333);  // �����ϰ��� �ϴ� ������ ��Ʈ�� �����Ѵ�.
	// ������ �ּҸ� �����Ѵ�.
	// �׽�Ʈ�� ����  ������ Ŭ���̾�Ʈ��
	// ���� ��ǻ�Ϳ��� ���� �����Ƿ�
	// 127.0.0.1�� �Ͽ� �� ��ǻ���� IP�� ����.
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	// Ŭ���̾�Ʈ�� server_address�� ������ ���� ������ ���� �Ѵ�.
	// �������� client ���Ϲ�ȣ�� �ش�.
	if(connect(format.server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1)
	{
		error("Connect() Error!");
	}
	
	printf("Hello '%s'!", format.name); // ȭ�鿡 ���������� �˷��ش�.
	printf("\n");
	write(format.server_socket, init_message, BUFFER_SIZE);	// ������ �ٸ� Ŭ���̾�Ʈ���� name�� ���������� �˷��ش�.
	
	
	pid = fork();	// ���μ����� �����Ѵ�.

	// pid�� ���� ���� �ؾ� �� �۾��� �����Ѵ�.
	switch(pid)
	{
		case -1:
			error("Fork() Error!");
			break;
		case 0:	// �۽� �۾�
			send_message(format);
			break;
		default: // ���� �۾�
			receive_message(format);
/*
				�θ� ���� ���ᰡ �ȴ�.
				������ �ڽ� ���μ����� fgets()���� �Է��� �ޱ⸦ ����Ѵ�.
				�̶� �θ�� �װ� �ڽ��� ��� �����Ƿ� �ڽ����μ����� �������μ����� �Ǵµ�
				�ڽ� ���μ����� ���̱� ���� �θ� ���μ����� �۾��� ������ kill �Լ��� �ڽ� ���μ����� ���δ�.
			*/
			kill(pid, SIGKILL);
			break;
	}
	


	
	printf("Server Shutdown!\n");
	close(format.server_socket);	// ������ �ݴ´�.
	exit(EXIT_SUCCESS);
}


// �޽��� �۽� �Լ�
void send_message(SOCKET_INFO socket_info)
{
	// Data data_format;	// �۽��� ������
	char message[1000];	// �۽��� �޽���
	char data_format[BUFFER_SIZE];	// ������ Ŭ���̾�Ʈ���� �۽��� ������ ���
	int server_flag;	// ���� ���� �ǵ�
	while(1)
	{

		// �޽����� 500�� �Է¹޴´�.(�ѱ��� 250��)
		fgets(message, 1000, stdin);
		message[(int) strlen(message)-1] = '\0';	// main�Լ����� �ߴ� ���๮�ڸ� �����ϴ� �۾��� �Ѵ�.
		sprintf(data_format, "[%s] : %s", socket_info.name, message);
		/*
		�̺κ��� q�� �Է� ���� �� �������� ������ ���� �κ�
		�ʿ� ���� �������� �Ǵ��Ͽ� �ּ� ó����
		if(!strcmp(message, "q"))
		{
			return;
		}
		*/
		server_flag = write(socket_info.server_socket, data_format, BUFFER_SIZE);
		switch(server_flag)
		{
			case -1:	// �б� ����
				error("Write() Error!");
			case 0:		// ������ ���� ���
				return;	// �Լ� �ߴ�
			default:	// ���������� �ۼ� �Ϸ�
				continue;
		}
	}
}

void receive_message(SOCKET_INFO socket_info)
{
	
	char data_format[BUFFER_SIZE];	// �������� ������ ������ ���	
	int server_flag;	// ���� ���� �ǵ�

	while(1)
	{
		server_flag = read(socket_info.server_socket, data_format, BUFFER_SIZE);	// ���Ͽ� �����͸� �д´�.
	//	printf("%d %d\n", read_flag, socket_info.server_socket);
		// ���� ���¸� ���� ó�� ���� 
		switch(server_flag)
		{
			case -1:	// �б� ����
				error("Read() Error!");
			case 0:	// ������ ���� ���
				return;
			default:	// �����Ͱ� ����
				printf("%s\n", data_format);	// �����͸� ���
		}
	}

}

void error(char* message)
{
	fputs(message, stderr);	// ���� �������� ����Ѵ�.
	putc('\n',stderr);
	exit(EXIT_FAILURE);
}

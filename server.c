
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
	int server_socket;	// 서버 소켓 파일 디스크립터
	int client_socket;	// 클러이언트 소켓 파일 디스크립터
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int client_address_size;	// 클라이언트 주소 크기
	pthread_t thread;
	int optval = 1; // 서버 강제 종료시 재시동 했을때 해당 포트에 바인딩을 다시 할 수 있다.
	int i;
					
	// 예외 처리와 통신을 위한 기본 설정
	// 뮤텍스 초기화 예외처리
	if(pthread_mutex_init(&mutex, NULL))
	{
		error("Muexex Init Error!");
	}

	//서버 소켓을 IPv4 인터넷 프로토콜 체계로 사용한다.
	//또한 텍스트 메시지를 보내므로 TCP/IP 프로토콜을 사용한다.
	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	
	// 서버 소켓을 생성 예외 처리
	if (server_socket == -1)
	{
		error("Socket() Error!");
	}

	// 서버를 재가동시 서버소켓이 기존 포트에 새롭게 바인딩을 할 수 있도록 설정한다.
	setsockopt(server_socket,SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	
	// 소켓 구조체에 내용을 0 또는 NULL 값으로 초기화
	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;	//서버 주소는 IPv4 인터넷 프로토콜 주소 쳬개로 설정한다.
	server_address.sin_port = htons(3333);	//서버의 포트 주소를 설정한다.
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);	// 현재 시스템의 서버 주소를 찾아 설정 한다.

	// 서버 소켓을 생성 예외 처리
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

	// 본격적으로 서버에서 클라이언트의 요청을 처리
	while(1)
	{
		client_address_size = sizeof(client_address);	//클라이언트의 주소 크기인 4Byte로 설정 

		// 클라이언트의 접속 요청을 수락하고
		// 해당 클라이언트의 파일 디스크립트 번호를 지정해준다.
		client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_size);

		// 클라이언트의 수락 실
		if (client_socket == -1)
		{
			error("Accept() Fail");
		}

		// 클라인언트의 소켓 스크립터를 기억한다.
		pthread_mutex_lock(&mutex);	// 이 작업중에는 다른 쓰레드는 작업을 할 수 없다.
		client_socket_array[client_number++] = client_socket;	// 클라이언트를 서버에 등록을 하고 클라이언트 접속자 수를 증가 시킨다.
//		printf("현재 접속자 %d명\n",client_number);	// 현재 접속자 수를 출력한다.
		pthread_mutex_unlock(&mutex);	// 이제 다른 쓰레드들이 다른 작업을 할 수 있게 한다.

		// thread를 생성한다.(옵션 : 기본값)
		// 쓰레드에 동작할 함수는 client_connection이며,
		// 클라이언트 소켓을 인자값으로 넘겨준다.
		// intptr_t는 int형 포인터 타입이며,
		// 컴파일 단계에서 포인터 타입에 맞춰주기 위해 intptr_t로 캐스팅연산후
		// void* 형으로 다시 캐스팅연산을 수행하여 넘겨줌.
		pthread_create(&thread, NULL, client_connection, (void *)(intptr_t)client_socket);

	}
	/***************************/

	exit(EXIT_SUCCESS);
}

// 클라이언트 연결 function
void* client_connection(void *arg)
{
	int client_socket = (intptr_t) arg;	// 소켓 값을 넘겨준다.
	int string_length = 0;	// 문자열 길이 확인
	char message[BUFFER_SIZE];	// 보낼 메시지
	char init_message[BUFFER_SIZE];
	int needless_string_length = 0;
	int i, j;	// 반복자

	// 메시시 수신
	while((string_length = read(client_socket, message, sizeof(message))) != 0)
	{
		/*
		   사용자가 채팅방에 입장 했을때와 입장 후 채팅하는 경우를 고려함
		   확률적으로 클라이언트는 채팅메시지를 입력 하는 경우가 많으므로 
		   서버는 우선적으로 첫번째 문자가 '['를 확인 하는 것이 서버 속도를
		   조금 더 빨리 할 수 있을 것이라 생각하여 다음과 같이 작성 하였다.
		 */

		
		if(message[0] == '[')	// 채팅 메시지를 입력 받은 경우
		{
			/*
			   서버는 클라이언트가 보낸 메시지가 몇글자를 입력했는지 알아하므로
			   개발자가 생각한 데이터 양식에 실제 클라이언트가 입력한 내용을 파싱한다.
			*/

			// 데이터 양식을 일단 끝까지 읽는다.
			for (i = 0; message[i] != '\0'; i++)
			{
				/*
				   이 작업도 역시 확률적으로 생각을 하여 작성하였다.
				*/

				if(message[i] == '[') // 데이터 양식의 첫번째 문자인겨우
				{
					
					needless_string_length++; // 불필요한 문자 수를 1증가 시킨다.
					continue;	// 밑에 문장을 무시하기 위해 사용 하였다.
				}
				else if(message[i] != ']')	// 사용자 이름인 경우
				{
					printf("%c",message[i]);	// 사용자 이름의 문자를 하나씩 출력한다.
					needless_string_length++;	// 1증가
					continue;	// 아래 조건문에 가지 않기 위해 처리
				}
				else	// 사용자의 이름의 범위를 알수있는 ']'문자 인경우
				{
					needless_string_length += 4;	// ' ', ':', ' ' 도 있기 때문에 그 문자도 포함 시켜준다.
					printf(" has enterd %d words\n", (int) strlen(message) - needless_string_length);	// 총 메시지 수를 작성한다.
					needless_string_length = 0;	// 새로운 메시지에 대해 초기화 해준다.
					break;	//원하는 결과를 얻었으므로 반복문을 빠져나온다.
				}		
			}
		}
		else	// 클라이언트가 채팅방에 입장한 경우 처리한다.
		{
			strcpy(user[client_number-1].name, message);	// 해당 클라이언트의 이름을 클라이언트 소켓 배열의 똑같은 인덱스에 저장한다.
			printf("Connected %s\n", message);	// 누가 접속했는지 서버에 알려준다.
			sprintf(init_message, "Connected %s", message);	// 위 문장을 다른 클라이언트에게 출력해준다.
			strcpy(message, init_message);
		}
		
		// 다른 클라이언트에게 받은 메시지를 보내준다.
		send_message(client_socket, message, string_length);
	}
	/*******************/
	
	/*
	   이 부분은 클라이언트가 연결이 종료 되면
	   서버와 다른 클라이언트에게 누가 종료 되었는지 알려주는 과정이다.
	*/

	pthread_mutex_lock(&mutex);	// 이 작업 중에는 다른 쓰레드는 사용할 수 없다.
	for(i = 0; i < client_number; i++)	// 클라이언트 수 만큼 반복한다.
	{
		if(client_socket == client_socket_array[i])	// 서버가 확인한 클라이언트가 종료된 클라이언트인 경우
		{
			// 서버에 어느 클라이언트가 접속을 끊었는지 출력한다.
			printf("%s 연결 종료\n", user[i].name);	

			// 서버는 다른 클라이언트에게 출력된 문장을 똑같이 보낼 준비를 한다.
			sprintf(message, "%s 연결 종료", user[i].name);

			// 클라이언트 수 만큼 반복한다.
			for(j = 0; j <client_number; j++)
			{
				// 접속을 끊은 클라이언트를 제외한
				// 다른 클라이언트에게 서버에 출력된 메시지를  보낸다.
				if(client_socket_array[j] != client_socket)
				{
					write(client_socket_array[j], message, sizeof(message));
				}
			}
			
			// 서버에 접속을 끊은 클라이언트를 제외 시키는 작업을 한다.
			for(; i < client_number - 1; i++)
			{
				client_socket_array[i] = client_socket_array[i+1];
				user[i] = user[i+1];
			}
			break;	// 해당 작업을 모두 수행 했으므로 반복문을 빠져 나온다.
		}
	}// end for()

	client_number--;	// 클라이언트의 수를 1감소 시킨다.
//	printf("현재 접속자 %d명\n",client_number); // 현재 접속자 수를 다시 확인한다.
	pthread_mutex_unlock(&mutex);	// 다른 쓰레드가 작업을 할 수 있게 해준다.
	/********************/

	// 소켓 종료
	close(client_socket);

	return 0;
}

// 메시지 송신
void send_message(int client_socket, char* message, int length)
{
	int i; // 반복자
	
	// 이 작업을 진행 중에는 다른 쓰레드에서 작업을 할 수 없다.
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

// 에러 메시지 출력
void error(char *message)
{
	fputs(message, stderr); // 표준에러에 해당 메시지를 출력한다.
	fputc('\n', stderr);	// 개행문도 추가한다.
	exit(EXIT_FAILURE);		// 서버가 가동이 실패했다고 OS에 알려 준다.
}

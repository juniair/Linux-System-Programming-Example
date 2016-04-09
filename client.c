
#include <stdio.h>  // using fputs(), fputc()
#include <stdlib.h> // using exit()
#include <unistd.h> // using close() 
#include <string.h> // using memset()
#include <ctype.h>	// using isalnum() -> 해당 문자가 영어 문자인지 숫자 문자인지 판독하는 함수
#include <signal.h> // using kiil()

// using socket(), bind(), listen(), accept(), htons(), htonl(), inet_ntoa()
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
 

#define BUFFER_SIZE 1024

// 구조체 정의
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
	SOCKET_INFO format;	//서버와 연결할 소켓 생성
	struct sockaddr_in server_address;	// 연결할 서버 정보 설정
	char init_message[BUFFER_SIZE];
	pid_t pid; // 부모는 수신을 자식은 송신을 해준다.
	int i;
	int name_flag;	// 사용자 이름이 규칙에 맞는지 판독 하는 flag

	// 클라이언트 사용자 설정
	// 사용자 명은 반드시 영어나 숫자이어야 한다.
	while(1)
	{
		printf("What your name? -> ");
		fgets(format.name, 11, stdin);
		format.name[strlen(format.name)-1] = '\0';	// fgets에서 입력된 개행문자를 null 문자로 변경
		
		// 규칙에 맞게 입력했는지 확인
		for(i = 0; format.name[i] != '\0'; i++)
		{
			if(isalnum(format.name[i]))	// 문자를 하나씩 판독한다.
			{
				name_flag = 0;	// 사용자 이름의 특수 문자가 없다.
			}
			else
			{
				name_flag = 1;	// 사용자 이름의 특수문자를 발견했다.
			}
		}
		
		if(name_flag)	// 이름의 이상이 있는가?
		{
			printf("Try Agine...\n");	// 재입력 시도
		}
		else
		{
			break;	// 다음 작업 수행
		}
	}
	sprintf(init_message, "%s", format.name);	// 서버에게 클라이언트의 이름을 보낸다.
	
	// 소켓을 IPv4 인터넷 프로토콜 체계로 사용한다.
	// 또한 텍스트 메시지만 전송이 되므로 TCP/IP 프로토콜을
	// 사용 해야 되므로 SOCK_STREAM을 사용한다.
	format.server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if(format.server_socket == -1)
	{
		error("Socket() Error!");
	}
	
	// server_address 구조체에 0 또는 NULL 값으로 초기화
	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;	// IPv4 인터넷 프로토콜 주소 체계로 설정한다.
	server_address.sin_port = htons(3333);  // 연결하고자 하는 서버의 포트를 지정한다.
	// 서버의 주소를 설정한다.
	// 테스트를 위해  서버와 클라이언트는
	// 같은 컴퓨터에서 돌고 있으므로
	// 127.0.0.1을 하여 내 컴퓨터의 IP를 얻어낸다.
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	// 클라이언트를 server_address의 정보를 가진 서버와 연결 한다.
	// 서버에는 client 소켓번호를 준다.
	if(connect(format.server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1)
	{
		error("Connect() Error!");
	}
	
	printf("Hello '%s'!", format.name); // 화면에 입장했음을 알려준다.
	printf("\n");
	write(format.server_socket, init_message, BUFFER_SIZE);	// 서버와 다른 클라이언트에게 name이 접속했음을 알려준다.
	
	
	pid = fork();	// 프로세스를 복제한다.

	// pid의 값에 따라 해야 될 작업을 결정한다.
	switch(pid)
	{
		case -1:
			error("Fork() Error!");
			break;
		case 0:	// 송신 작업
			send_message(format);
			break;
		default: // 수신 작업
			receive_message(format);
/*
				부모가 먼저 종료가 된다.
				하지만 자식 프로세스는 fgets()에서 입력을 받기를 대기한다.
				이때 부모는 죽고 자식은 살아 있으므로 자식프로세스는 좀비프로세스가 되는데
				자식 프로세스를 죽이기 위해 부모 프로세스에 작업이 끝나면 kill 함수로 자식 프로세스를 죽인다.
			*/
			kill(pid, SIGKILL);
			break;
	}
	


	
	printf("Server Shutdown!\n");
	close(format.server_socket);	// 소켓을 닫는다.
	exit(EXIT_SUCCESS);
}


// 메시지 송신 함수
void send_message(SOCKET_INFO socket_info)
{
	// Data data_format;	// 송신할 데이터
	char message[1000];	// 송신할 메시지
	char data_format[BUFFER_SIZE];	// 서버와 클라이언트에게 송신할 데이터 양식
	int server_flag;	// 서버 상태 판독
	while(1)
	{

		// 메시지를 500자 입력받는다.(한글은 250자)
		fgets(message, 1000, stdin);
		message[(int) strlen(message)-1] = '\0';	// main함수에서 했던 개행문자를 제거하는 작업을 한다.
		sprintf(data_format, "[%s] : %s", socket_info.name, message);
		/*
		이부분은 q를 입력 했을 때 서버와의 연결을 끝는 부분
		필요 없는 동작으로 판단하여 주석 처리함
		if(!strcmp(message, "q"))
		{
			return;
		}
		*/
		server_flag = write(socket_info.server_socket, data_format, BUFFER_SIZE);
		switch(server_flag)
		{
			case -1:	// 읽기 실패
				error("Write() Error!");
			case 0:		// 서버가 죽은 경우
				return;	// 함수 중단
			default:	// 성공적으로 작성 완료
				continue;
		}
	}
}

void receive_message(SOCKET_INFO socket_info)
{
	
	char data_format[BUFFER_SIZE];	// 서버에서 수신할 데이터 양식	
	int server_flag;	// 서버 상태 판독

	while(1)
	{
		server_flag = read(socket_info.server_socket, data_format, BUFFER_SIZE);	// 소켓에 데이터를 읽는다.
	//	printf("%d %d\n", read_flag, socket_info.server_socket);
		// 읽은 상태를 보고 처리 결정 
		switch(server_flag)
		{
			case -1:	// 읽기 실패
				error("Read() Error!");
			case 0:	// 서버가 죽은 경우
				return;
			default:	// 데이터가 존재
				printf("%s\n", data_format);	// 데이터를 출력
		}
	}

}

void error(char* message)
{
	fputs(message, stderr);	// 오류 형식으로 출력한다.
	putc('\n',stderr);
	exit(EXIT_FAILURE);
}

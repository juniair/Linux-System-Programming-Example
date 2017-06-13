#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/stat.h> 
#include <fcntl.h>
#define BUFSIZE 1024

typedef struct mymsg{
	int size;
	char name[BUFSIZE];
}msg;

typedef struct ap{
	long off;
	char name[BUFSIZE];
}app;

void err_quit(char *msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
int recvn(int s, char *buf, int len) {
	int received, left = len;
	char *ptr = buf;

	while (left > 0) {
		received = read(s, buf, left);
		if (received == -1)
			return -1;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}
	return (len - left);
}

int main(int argc, char *argv[])
{
	int retval, sock;
	msg* list;
	list = (msg*)malloc(sizeof(msg));
	app* append;
	append = (app*)malloc(sizeof(app));
	struct sockaddr_in serveraddr;
	struct dirent*direntp;
	direntp = malloc(sizeof(struct dirent));
	char select[2];
	int fd;
	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	retval = connect(sock, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
	while (1)
	{
		fflush(stdout);
		fflush(stdin);
		printf("�޴� ���� (1. ��� 2.�ٿ�ε� 3. ����) : ");
		scanf("%s", select);
		write(sock, select, 2);
		char buf[BUFSIZE];
		if (!strcmp(select, "1"))
		{
			int totalbytes;
			while (1)
			{
				retval = read(sock, buf, sizeof(msg));
				memcpy(list, buf, sizeof(msg));
				if (list->size == -1){
					break;
				}
				printf("���� �̸� : %s \t���� ũ�� : %d\n", list->name, list->size);

			}
			
		}
		else if (!strcmp(select, "2"))
		{
			char filename[256];
			char ssend[BUFSIZE];

			memset(filename, 0, sizeof(filename));
			printf("�ٿ�ε� �� ���� �̸� : ");
			fflush(stdin);
			scanf("%s", filename);
			fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			if (fd < 0) {
				err_quit("���� I/O ����");
				close(sock);
				return 1;
			}
			strcpy(append->name, filename);
			memcpy(ssend, append, sizeof(app));
			retval = write(sock, ssend, sizeof(app));
			if (retval == -1) {
				err_quit("recv() ����");
				close(sock);
				return 1;
			}

			int totalbytes;
			retval = recvn(sock, (char *)&totalbytes, sizeof(totalbytes));
			if (retval == -1) {
				err_quit("recv() ����");
				close(sock);
				return 1;
			}
			printf("%d\n", totalbytes);
			int numtotal = 0;
			int sum = 0;
			int nwrite = 0;
			while (1) {
				sum = numtotal;
				if (sum == totalbytes){
					fprintf(stderr, "com\n");
					break;
				}
				char readbuf[BUFSIZE] = { 0 };
				retval = read(sock, readbuf, BUFSIZE);
				if (retval == -1) {
					err_quit("recv() ����");
				}
				else {
					nwrite = write(fd, readbuf, retval);
					numtotal += nwrite;
				}
			}
			close(fd);
			if (numtotal == totalbytes)
				fprintf(stderr, "-> ���� ��ȯ ����\n");
			else
				fprintf(stderr, "-> ���� ��ȯ ����\n");
			fprintf(stderr, "���� �ޱ� ����\n");


		}

		else if (!strcmp(select, "3"))
		{
			write(sock, select, 2);
			printf("���α׷� ����\n");
			break;
		}
		else
		{
			printf("�߸��� �޴�\n");
		}
	}
	return 0;
}

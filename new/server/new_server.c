#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h> 
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>

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
 
void addClient(int s, struct sockaddr_in *newcliaddr){
	char buf[20];

	inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf));
	printf("new client: %s\n", buf);
}

int recvn(int s, char *buf, int len) {
	int received, left = len;
	char *ptr = buf;
 
	while(left > 0) {
		received = read(s, buf, left);
		if(received == -1)
			return -1;
		else if(received == 0)
			break;
		left -= received;
		ptr += received;
	}
 
	return (len - left);
}

void* WorkerThread(void *Arg){
	int retval, client_sock = (int)Arg;
	off_t endoff;
 	msg* list;
	list = (msg*) malloc(sizeof(msg));
	app* append;
	append = (app*)malloc(sizeof(app));
 	struct stat file_info;
	struct sockaddr_in serveraddr;
	DIR * dirp;
	struct dirent * direntp;
	int addrlen;

	char buf[BUFSIZE];
 	char menu[2];
	int fd;

	while(1)

	{
	retval = recvn(client_sock,menu,2);
	if(!strcmp(menu,"2"))

	{
 	char filename[256];
	char ssend[BUFSIZE];
	retval = recvn(client_sock, ssend, sizeof(app));

	memcpy(append, ssend, sizeof(app));

	if(retval == -1)
		err_quit("write() error 1");
	printf("recived name : %s\n", append->name);
	fd = open(append->name,O_RDONLY);
	if(fd < 0) 
		err_quit("fopen() error");

	
	int totalbytes=lseek(fd,0,SEEK_END);
	
	retval = write(client_sock, (char *)&totalbytes, sizeof(totalbytes));

 
	char buf[BUFSIZE]={0};

	int numread = 0, sum = 0, numtotal=0;
	lseek(fd, 0, SEEK_SET);
	char writebuf[BUFSIZE];
	
	while(1) {

		numread = read(fd,writebuf,BUFSIZE);
		if(numtotal == totalbytes){
			printf("파일 전송 완료 : %d bytes\n", numtotal);
			break;
		}

		else if(numread > 0) {
			retval = write(client_sock,writebuf,numread);
			if(retval == -1)
				err_quit("write() error!");
			numtotal += retval;
		}

		else {
			err_quit("file I/O error");
		}

	}
	close(fd);
 

	}
	else if(!strcmp(menu,"3"))
	{

 	char filename[256];
	char ssend[BUFSIZE];

	retval = recvn(client_sock, ssend, sizeof(app));
	memcpy(append, ssend, sizeof(app));

	if(retval == -1)
		err_quit("write() error");
	fd = open(append->name, O_RDWR|O_APPEND);

	if(fd < 0) 
		err_quit("fopen() error");
 
	int totalbytes=lseek(fd, 0, SEEK_END);
	retval = write(client_sock, (char *)&totalbytes, sizeof(totalbytes));

	if(retval == -1)
		err_quit("write() error");

 
	char buf[BUFSIZE];
	int numread=0;

	int numtotal = 0;
 	if(totalbytes == append->off){
		printf("이미 다 받으셨습니다.\n");
		buf[0] = -100;
		write(client_sock, buf, sizeof(buf));
	}

	lseek(fd, 0, SEEK_SET);
	lseek(fd, append->off-1, SEEK_SET);
	int sum=0;
	while(1) {
		char writebuf[BUFSIZE]={0};
		sum=append->off-1 + numtotal;
		numread = read(fd,writebuf, BUFSIZE);
		if(sum== totalbytes) {

			printf("파일 전송 완료 : %d bytes\n", numtotal);
			break;
		}

		else if(numread > 0) {
			retval = write(client_sock,writebuf,numread);
			numtotal += numread;
		}

		else {
			err_quit("file I/O error");
		}

	}
	close(fd);


	}
		else if(!strcmp(menu,"1"))
		{

			int ptr;

			dirp = opendir("./");
			while ((direntp = (struct dirent*)readdir(dirp)) != NULL){
					stat(direntp->d_name, &file_info);

					printf("\n");
					list->size = file_info.st_size;
					memcpy(list->name, direntp->d_name, sizeof(direntp->d_name));

					memcpy(buf, list, sizeof(msg));
					write(client_sock, buf, sizeof(msg));
			}
			if(direntp == NULL){
				list->size = -1;
				memcpy(buf, list, sizeof(msg));
				write(client_sock, &buf, sizeof(msg));
			}

		}
		else if(!strcmp(menu,"4"))
		{

			printf("연결안됨\n");
			close(client_sock);
			break;

		}
	}

}
 
int main(int argc, char *argv[])
{
	int retval;
	int maxfdp1 = 101; 
	off_t endoff;
 	msg* list;
	list = (msg*) malloc(sizeof(msg));
	app* append;
	append = (app*)malloc(sizeof(app));
	int listen_sock = socket(PF_INET, SOCK_STREAM, 0);

 	struct stat file_info;
	struct sockaddr_in serveraddr;
	DIR * dirp;
	struct dirent * direntp;
	fd_set read_fds;

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
	retval = listen(listen_sock, 5);

	int client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE];
 	char menu[2];
	while(1) {
		pthread_t tid;
		tid = pthread_self();
		FD_ZERO(&read_fds);
		FD_SET(listen_sock, &read_fds);
		if(select(maxfdp1, &read_fds, NULL, NULL, NULL)<0){
			perror("error : ");
		}
		if(FD_ISSET(listen_sock, &read_fds)){
			addrlen = sizeof(clientaddr);
			client_sock = accept(listen_sock, (struct sockaddr*) &clientaddr, &addrlen);
			if(client_sock == -1)
				err_quit("accept() error");
 
			printf("\n->FileSender connect : IP = %s, Port = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			addClient(client_sock, &clientaddr);
		}
	pthread_create(&tid, NULL, WorkerThread, client_sock);
	pthread_detach(tid);
	} 
	close(listen_sock);
 
	return 0;
 
}


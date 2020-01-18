#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#define HSTATIC_PORT 8080
#define HSTATIC_TCP_BACKLOG 4
#define EOL "\r\n"
#define EOL_SIZE 2

typedef struct server {
	int listen_fd;
} server_t;

int server_listen(server_t* server){
	int err = 0;
	struct sockaddr_in server_addr = { 0 };
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port        = htons(HSTATIC_PORT);
	err = (server->listen_fd = socket(AF_INET, SOCK_STREAM, 0));
	if (err == -1) {
		perror("socket");
		printf("Failed to create socket endpoint\n");
		return err;
	}
	err = bind(server->listen_fd,
	           (struct sockaddr*)&server_addr,
	           sizeof(server_addr));
	if (err == -1) {
		perror("bind");
		printf("Failed to bind socket to address\n");
		return err;
	}
	err = listen(server->listen_fd, HSTATIC_TCP_BACKLOG);
	if (err == -1) {
		perror("listen");
		printf("Failed to put socket in passive mode\n");
		return err;
	}
}

char* get_file_path(char* url){ //Get the correct path of the requested file in the server
	if(strcmp(url,"/") == 0 || strcmp(url,"/home.html") == 0 || strcmp(url,"") == 0){
		return "html/home.html";
	}
	else if(strcmp(url,"/a.html") == 0 ){
		return "html/a.html";
	}
	else if(strcmp(url,"/b.html") == 0 ){
		return "html/b.html";
	}
	else if(strcmp(url,"/c.html") == 0 ){
		return "html/c.html";
	}
	return "html/404.html";
}

char* get_requested_url(int conn_fd){ //To get the requested url in HTTP get request
	char body[99999], *line[3];
	memset((void*)body,(int)'\0', 99999);
	int rs = recv(conn_fd, body, 99999, 0);
	line[0] = strtok(body, " \t\n");
	if(strncmp(line[0], "GET\0", 4) == 0){
		line[1] = strtok(NULL, " \t");
		return line[1];
	}
	return "";
}

int send_file(int conn_fd, char* url){
	char *reply = "";
	char* url1 = "";
	char send_buffer[1000];
	int sz = 0;
	url1 = get_file_path(url);
	FILE *sendFile = fopen((url1), "r"); //Always there will be a file returned by get_file_path.
	if(strcmp(url1,"html/404.html") == 0){
		printf("File not found!\n");
		sendFile = fopen("html/404.html", "r");
		fseek(sendFile, 0L, SEEK_END);
		sz = ftell(sendFile);
		char header[102400];
		sprintf(header, 
			"HTTP/1.1 404\n"
			"Content-Type: text/html\n"
			"Connection: Keep-Alive\n"
			"Content-Length: %i\n"
        		"\n", sz);
		send(conn_fd, header, strlen(header), 0);
	}else{
		printf("File found. Sending now...\n");
		fseek(sendFile, 0L, SEEK_END);
		sz = ftell(sendFile);
		char header[102400];
		sprintf(header, 
			"HTTP/1.1 200 OK\n"
			"Content-Type: text/html\n"
			"Connection: Keep-Alive\n"
			"Content-Length: %i\n"
        		"\n", sz);
		send(conn_fd, header, strlen(header), 0);
	}
	sendFile = fopen((url1), "r");
	while( !feof(sendFile) )
	{
		int numread = fread(send_buffer, sizeof(unsigned char), 1000, sendFile);
		if( numread < 1 ) break; // EOF or error
		char *send_buffer_ptr = send_buffer;
		int numsent = send(conn_fd, send_buffer_ptr, numread, 0);
		send_buffer_ptr += numsent;
    	numread -= numsent;
	}
	return 0;
}

int server_accept(server_t* server){
	int err = 0;
	int conn_fd;
	socklen_t client_len;
	struct sockaddr_in client_addr;
	client_len = sizeof(client_addr);
	err =
	  (conn_fd = accept(
	     server->listen_fd, (struct sockaddr*)&client_addr, &client_len));
	char *s = inet_ntoa(client_addr.sin_addr);
	printf("HTTP request recieved from IP address: %s\n", s);
	if (err == -1) {
		perror("accept");
		printf("Failed accepting connection\n");
		return err;
	}
	printf("HTTP Request processing now...\n");
	
	send_file(conn_fd, get_requested_url(conn_fd));
	err = close(conn_fd);
	if (err == -1) {
		perror("close");
		printf("failed to close connection\n");
		return err;
	}
	return err;
}

int main()
{
	int err = 0;
	server_t server = { 0 };
	err = server_listen(&server);
	if (err) {
		printf("Failed to listen on address 0.0.0.0:8080");
		return err;
	}
	for (;;) {
		if(fork() == 0){
			err = server_accept(&server);
			if (err) {
				printf("Failed accepting connection\n");
				return err;
			}
		}
		
	}
	return 0;
}

#include <stdio.h>
#include <string.h>	
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>
#include <ctype.h>


#define html "text/html"
#define mp3 "audio/mpeg"
#define mp4 "video/mp4"
#define jpg "image/jpeg"
#define jpeg "image/jpeg"
#define png "image/png"
#define txt "text/plain"

int port = 4200;
char *mime_type;


void send_responce(int fd, char *header, void *body,int contentLength) {
  
  char response[contentLength+100]; 
  int response_length = sprintf(response,
                "%s\n"
                "Connection: close\n"
                "Content-Length: %d\n"
                "Content-Type: %s\n"
                "\n",
                header,
                contentLength, 
                mime_type);

  memcpy(response + response_length, body, contentLength);
  send(fd, response, response_length + contentLength, 0);

}

void send_file(int fd, char *file_name){
    
    
    char *source;
    FILE *file = fopen(file_name, "r");
    size_t bufsize;
    if (file != NULL) {
        if (fseek(file, 0L, SEEK_END) == 0) {
            bufsize = ftell(file);
            if(bufsize > 10000000){
                char *data = "size limit exists";
                send_responce(fd, "HTTP/1.1 500 Internal Server Error", data, strlen(data));
                return;
            }
            source = malloc(sizeof(char) * (bufsize + 1));
            fseek(file, 0L, SEEK_SET);    
            fread(source, sizeof(char), bufsize, file);
            send_responce(fd,"HTTP/1.1 200 OK", source, bufsize);
        }
        free(source);
        fclose(file);
    }else{

        char *error = "file not found";
        mime_type = html;
        send_responce(fd, "HTTP/1.1 404 NOT FOUND", error, strlen(error));
    }
  
}

int main(int argc , char *argv[])
{
    if(argc>=2) { 
        char *next;
        int port_number = strtol (argv[1], &next, 10);
        if ((next == argv[1]) || (*next != '\0')) {
            port = 4200;
        } else {
            port = port_number;
        }
    } 

	int server_fd, client_fd;
	struct sockaddr_in server;
    char buffer[1024] = {0}; 
    char requestType[4];       
    char requestpath[1024];    

	
	server_fd = socket(AF_INET , SOCK_STREAM , 0);
	if (server_fd == -1){
		perror("not create socket");
        return -1;
	}
		
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if(bind(server_fd, (struct sockaddr *)&server , sizeof(server)) == -1){
        perror("bind fail");
        return -1;
    }

    if(listen(server_fd,1) == -1){
        perror("listen fail");
        return -1;
    }

    printf("serve listen on port %d\n\n", port);
    while (1){
        
        if((client_fd = accept(server_fd, (struct sockaddr*)&server,(socklen_t*)&server)) == -1){
            perror("accept fail");
            return -1;
        }  

	
        read( client_fd , buffer, 1024); 
    
        
        
        sscanf(buffer, "%s %s", requestType, requestpath);
        printf("requestType %s \n", buffer); 
	
        
        for(int i = 0; i < 100; i++)
            requestpath[i] = tolower(requestpath[i]);
        
        char *mime = strrchr(requestpath, '.')+1;
        char *name = strtok(requestpath, "/");
        
        if(mime)
            mime_type = mime;
        else
            mime_type = NULL;

        if (!strcmp(requestType, "GET") && !strcmp(requestpath, "/")) {
            char *data = "Please insert get request";
            mime_type = html;
            send_responce(client_fd, "HTTP/1.1 200 OK", data, strlen(data));
        }else if (!strcmp(requestType, "POST") && !strcmp(requestpath, "/")) {
            char *data = "hello post request";
            mime_type = html;
            send_responce(client_fd, "HTTP/1.1 200 OK", data, strlen(data));
        }else{
            send_file(client_fd,name);
        }  
        
    }

    close(client_fd);
	close(server_fd);
	return 0;
}

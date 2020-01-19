#include <stdio.h>
#include <string.h>	
#include <stdlib.h>
#include <ctype.h>	
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>

#define html "text/html"
#define mp3 "audio/mpeg"
#define mp4 "video/mp4"
#define jpg "image/jpeg"
#define jpeg "image/jpeg"
#define png "image/png"
#define txt "text/plain"

int port = 8080;
char *mime_type;


void send_res(int fd, char *header, void *body,int file_Len) {
  
  char res[file_Len+100]; 
  int res_Len = sprintf(res,
                "%s\n"
                "Connection: close\n"
                "Content-Length: %d\n"
                "Content-Type: %s\n"
                "\n",
                header,
                file_Len, 
                mime_type);

  memcpy(res + res_Len, body, file_Len);
  send(fd, res, res_Len + file_Len, 0);

}

void send_file(int fd, char *file_name){
    
    
    char *source;
    FILE *file = fopen(file_name, "r");
    size_t bufsize;
    if (file != NULL) {
        if (fseek(file, 0L, SEEK_END) == 0) {
            bufsize = ftell(file);
            if(bufsize > 10000000){
                char *data = "Size limit exceeds";
                send_res(fd, "HTTP/1.1 500 Internal Server Error Occured...", data, strlen(data));
                return;
            }
            source = malloc(sizeof(char) * (bufsize + 1));
            fseek(file, 0L, SEEK_SET);    
            fread(source, sizeof(char), bufsize, file);
            send_res(fd,"HTTP/1.1 200 OK", source, bufsize);
        }
        free(source);
        fclose(file);
    }else{

        char *error = "File not found";
        mime_type = html;
        send_res(fd, "HTTP/1.1 404 NOT FOUND", error, strlen(error));
    }
  
}

int main(int argc , char *argv[])
{
    if(argc>=2) { 
        char *next;
        int port_number = strtol (argv[1], &next, 10);
        if ((next == argv[1]) || (*next != '\0')) {
            port = 8080;
        } else {
            port = port_number;
        }
    } 

	int server_fd;
    int client_fd;
	struct sockaddr_in server_addr;
    char buffer[1024] = {0}; 
    char requestType[4];       
    char requestpath[1024];    

	
	server_fd = socket(AF_INET , SOCK_STREAM , 0);
	if (server_fd == -1){
		perror("not create socket");
        return -1;
	}
		
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if(bind(server_fd, (struct sockaddr *)&server_addr , sizeof(server_addr)) == -1){
        perror("bind fail");
        return -1;
    }

    if(listen(server_fd,1) == -1){
        perror("listen fail");
        return -1;
    }

    printf("Server is listening on port %d\n\n", port);
    while (1){
        
        if((client_fd = accept(server_fd, (struct sockaddr*)&server_addr,(socklen_t*)&server_addr)) == -1){
            perror("Accept fail");
            return -1;
        }  

	
        read( client_fd , buffer, 1024); 
    
        
        
        sscanf(buffer, "%s %s", requestType, requestpath);
        printf("RequestType %s \n", buffer); 
	
        
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
            send_res(client_fd, "HTTP/1.1 200 OK", data, strlen(data));
        }else if (!strcmp(requestType, "POST") && !strcmp(requestpath, "/")) {
            char *data = "hello post request";
            mime_type = html;
            send_res(client_fd, "HTTP/1.1 200 OK", data, strlen(data));
        }else{
            send_file(client_fd,name);
        }  
        
    }

    close(client_fd);
	close(server_fd);
	return 0;
}

//server side implementation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ERR -1
#define MAX_CLIENTS 2
#define MAX_DATA 1024

main(int argc, char **argv) {
  struct sockaddr_in remote_server ;
  struct sockaddr_in server;
  struct sockaddr_in client;
  int sock;
  int new;
  int sockaddr_len = sizeof( struct sockaddr_in ) ;
  int data_len;
  char data[MAX_DATA];

  if((sock = socket ( AF_INET, SOCK_STREAM, 0)) == ERR ){
    perror("server socket: ");
    exit(-1);
  }

  remote_server.sin_family =  AF_INET ;
	remote_server.sin_port = htons( atoi ( argv[1] ) ) ;
	remote_server.sin_addr.s_addr = INADDR_ANY ;
	bzero( &remote_server.sin_zero, 8) ;

  if(bind (sock, (struct sockaddr *) &server, sockaddr_len) == ERR ) {
    perror("bind : ") ;
    exit(-1) ;
  }

  if(listen (sock, MAX_CLIENTS) == ERR){
    perror("listen") ;
    exit(-1) ;
  }

  while (1) {
    if( (new = accept (sock, (struct soockaddr *)&client, &sockaddr_len)) == ERR ){
      data_len = 1;

      while (data_len) {
        data_len = recv(new, data,  MAX_DATA, 0) ;

        if (data_len) {
          send(new, data, data_len, 0) ;
          data[data_len] = '\0' ;
          printf("Sent message : %s", data);
        }
      }
      printf("Client Disconnected...\n") ;
      close(new);
    }
    close(sock) ;
  }
}

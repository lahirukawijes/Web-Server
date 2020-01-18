#include  <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define html "text/html"
#define text "text/plain"
#define mp3 "audio/mpeg"
#define mp4 "video/mp4"
#define jpeg "image/jpeg"
#define png "image/png"
#define HSTATIC_PORT 8080
#define HSTATIC_TCP_BACKLOG 4
#define EOL "\r\n"
#define EOL_SIZE 2

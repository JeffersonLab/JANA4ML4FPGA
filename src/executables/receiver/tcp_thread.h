#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
/*=====================================================================*/
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

int tcp_open_th(int PORT,char* hostname);
int open_remote_th(int PORT,char* hostname);
int tcp_send_th(int sd_current,int *DATA,int lenDATA);
int tcp_get_th(int sd_current,int *DATA,int lenDATA);
int tcp_listen3(int sd,char* host_name, int len, int *sd_current);

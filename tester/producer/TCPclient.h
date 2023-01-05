//#define JF_WIN 1

  #include  <stdio.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <errno.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <signal.h>
  #include <stdlib.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <errno.h>
  #include <sys/wait.h>
  #include <time.h>



//#define PACKSIZE     65536
//#define PACKSIZE     131072
#define PACKSIZE     400111

#define DEBUG   2
#define T_WAIT  1

//---  REPLACE with your server machine name  
//-- !! NOT USED fsv since 14.01.2008
//
//#define HOST        "137.138.65.180"
//#define HOST        "137.138.65.34"
//#define HOST        "131.220.165.146"  //-- julia bonn
//#define HOST    "131.220.163.237"        //-- sf bonn
//#define HOST        "localhost"
//#define PORT        20248

#define DIRSIZE     8192
#define SDDIRSIZE     16384


int tcp_event_pack( unsigned int *DATA, int lenDATA,int n,int k, unsigned int evtHDR, unsigned int TriggerID );
int tcp_event( unsigned int *DATA, int lenDATA ,int n, int i, unsigned int evtHDR, unsigned int TriggerID);
int tcp_event_host(char *host, int port);
int tcp_event_get_sd();
int tcp_event_hold(int hflag);
//-- for tcp_main ---

int tcp_event_snd(unsigned int *DATA, int lenDATA ,int n, int i, unsigned int evtHDR, unsigned int Trig);
int tcp_event_get(char *HOST, unsigned int *DATA, int *lenDATA, int *Nr_Mod ,int *ModuleID, unsigned int *Trig);

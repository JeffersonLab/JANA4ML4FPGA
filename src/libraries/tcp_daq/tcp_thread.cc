#include "tcp_thread.h"
//struct sockaddr_in sinn;
//struct sockaddr_in pin;
//struct hostent *gethostbyname(char *);
//struct hostent *hp;
//int	sd, sd_current;
extern int sig_int, sig_hup, sig_alarm, sig_pipe;   
/*====================================================================*/
/*             TCP open                                               */
/*====================================================================*/
int tcp_open_th(int PORT,char* hostname){
  int sd=0;
  printf(" tcp_open:: PORT=%d HOST=%s \n",PORT,hostname);
  if (PORT<0) { PORT=-PORT; sd=open_remote_th(PORT,hostname); }
  else        {             sd=open_local_th(PORT);  }
  return sd;
}
/*--------------------------------------------------------------------*/
int open_local_th(int PORT){                      /*--- LISTEN !!! -----*/
    int on = 1;
    int sd =0;
    struct sockaddr_in sinn;
    printf(" open_local:: PORT=%d \n",PORT);
    /* get an internet domain socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socket");
	return -1;
    }
    printf("tcplib::open_local:: setsockopt(sock=%d) SO_REUSEADDR  !!!\n",sd);
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
	perror("setsockopt(SO_REUSEADDR) failed");
    }
    
    /* complete the socket structure */
    memset(&sinn, 0, sizeof(sinn));
    sinn.sin_family = AF_INET;
    sinn.sin_addr.s_addr = htonl(INADDR_ANY);
    sinn.sin_port = htons(PORT);

    printf("tcp_thread::open_local::  bind socket=%d  !!!\n",sd);
    /* bind the socket to the port number */
    if (bind(sd, (struct sockaddr *) &sinn, sizeof(sinn)) == -1) {
	perror("bind");
	return -1;
    }
    return sd;
}
/*--------------------------------------------------------------------*/
int open_remote_th(int PORT,char* hostname){   /*----- CONNECT  !!!  ----*/
  struct sockaddr_in pin;
  struct hostent *hp;
  int	sd_current;
  if(!sig_hup) printf(" go find out about the desired host machine \n");
  if ((hp = gethostbyname(hostname)) == 0) {
    if(!sig_hup) perror("gethostbyname");
    return -1;
  }
  if(!sig_hup) printf(" name=%s \n",hp->h_name);
  if(!sig_hup) printf(" alias=%s \n",(char*)hp->h_aliases);
  if(!sig_hup) printf(" adr_type=0x%x \n",hp->h_addrtype);
  if(!sig_hup) printf(" adr_len=%d \n",hp->h_length);
  if(!sig_hup) printf(" address=%s \n",hp->h_addr);
  
  if(!sig_hup) printf(" fill in the socket structure with host information \n");
  //        sprintf(hp->h_addr,"131.169.46.71");
  
  /* fill in the socket structure with host information */
  memset(&pin, 0, sizeof(pin));
  
  if(!sig_hup) printf(" after memset info \n");
  
  pin.sin_family = AF_INET;
  pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
  pin.sin_port = htons(PORT);

  if(!sig_hup) printf(" grab an Internet domain socket\n ");
  
  if ((sd_current = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
    if(!sig_hup) perror("socket");
     return -1;
  }
  
  if(!sig_hup) printf(" get socket option \n ");
  // getsockopt(sd,0,SO_SNDBUF,optval,optlen);           
  
  // printf("optval,optlen=  %d  %d \n",*optval,*optlen);
  //printf("optval,optlen=  %s  %d \n",*optval,*optlen);
  
  
  if(!sig_hup) printf(" try to connect \n");
  /* connect to PORT on HOST */
  if (connect(sd_current,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
    if(!sig_hup) perror("connect");
    return -1;
  }
  return sd_current;
}


/*====================================================================*/
/*             TCP send    data    2                                  */
/*====================================================================*/
int tcp_send_th(int sd_current,int *DATA,int lenDATA){
   int nleft,nsent;
   char* snd;
  //-----------------------------------------------------
   nleft=lenDATA;
   snd=(char*)DATA;
   while(nleft>0){
       // printf("try to send = %d of %d\n",PACKSIZE,nleft);
       nsent=send(sd_current,snd, nleft, 0);
       if (nsent <=0) { perror("send"); return -1;}
       nleft-=nsent;
       snd+=nsent;
   }
   return nleft; 
}

/*====================================================================*/
int tcp_get_th(int sd_current,int *DATA,int lenDATA){
  int nleft,nread=0;        
  char *rcv;
      //-----------------------------------------------------
      //if(!sig_hup)  printf("tcp_get_th(sd_current=%d DATA=%d lenDATA=%d)\n",sd_current,DATA[0],lenDATA);
      nleft=lenDATA;
      rcv=(char*)DATA;
      while(nleft>0){
        //if(!sig_hup) printf("tcp_get_th:: try to receive = %d of %d\n",nleft,lenDATA);
	nread=recv(sd_current,rcv,nleft, 0);
	if (nread <=0) {perror("recv"); return -1;}
	nleft-=nread;
        //if(!sig_hup)  printf("tcp_get_th:: nread= %d ==== nleft=%d\n",nread,nleft);
	rcv+=nread;
      }
      return nleft; 
}

/*====================================================================*/
int tcp_listen3(int sd, char* host_name, int len, int *sd_current){
  struct sockaddr_in pin;
  struct hostent *hp;
  socklen_t 	 addrlen;
  int rem_port;
  printf("Waiting for replay\n");
  /* show that we are willing to listen */
  if (listen(sd, 5) == -1) {
    perror("listen");
    return -1;
  }
  /* wait for a client to talk to us */
  addrlen=sizeof(pin);
  if ((*sd_current = accept(sd, (struct sockaddr *)  &pin, &addrlen)) == -1) {
    perror("accept");
    return -1;
  }
  rem_port=ntohs(pin.sin_port);
  printf("listen:: hp->h_name:: rem_port=%d \n",rem_port);

  printf("listen:: try get host by addr %s \n",inet_ntoa(pin.sin_addr));

  if ((  hp = gethostbyaddr(&pin.sin_addr,sizeof(pin.sin_addr),AF_INET)    ) == 0) {
      perror("tcp_listen3()::gethostbyaddr");
      strncpy(host_name,inet_ntoa(pin.sin_addr),len); host_name[len-1]=0;
  } else { 
      printf("listen3:: hp->h_name:: ptr=%p \n",hp->h_name);   
      strncpy(host_name,hp->h_name,len);              host_name[len-1]=0;
  }
  printf("listen3:: Connection from %s(%s) accept at sock=%d rem_port=%d\n"
	 ,host_name,inet_ntoa(pin.sin_addr),*sd_current,rem_port);
  return rem_port;

}
/*==================================================================*/

#include "TCPclient.h"
static char HOST[128];
static int  PORT=20249;
static int TCP_FLAG;
static int current_sd=0;
static int HOLD_TIME=0; //--- assume only one sender (DC) !!!

//-------------------------------------------------------------------------------------------------------
int tcp_event_host(char *host, int port) {    
    strncpy(HOST,host,128);
    PORT=port;
    printf("==>> TCPclient:: set HOST=%s PORT=%d \n",HOST,PORT);
}
//-------------------------------------------------------------------------------------------------------
int tcp_event_get_sd() {    
  printf("==>> TCPclient:: set HOST=%s PORT=%d SD=%d \n",HOST,PORT,current_sd);
  return current_sd;
}
//-------------------------------------------------------------------------------------------------------
int tcp_event_hold(int htime) {    
  printf("==>> TCPclient:: HOLD TIME = %d new=%d\n",HOLD_TIME,htime);
  if (HOLD_TIME==0 || htime==0) HOLD_TIME=htime;
  return HOLD_TIME;
}
//-------------------------------------------------------------------------------------------------------
int tcp_get3(int sock,int *DATA,int lenDATA){
    int nleft,nread=0;        
    char *rcv;
    nleft=lenDATA;
    rcv=(char*)DATA;
    while(nleft>0){
//      if(!sig_hup) printf("try to receive = %d of %d\n",nleft,lenDATA);
        nread=recv(sock,rcv,nleft, 0);
        if (nread <=0) {perror("tcp_get3():recv"); return -1;}
        nleft-=nread;
//        if(!sig_hup)  printf("nread= %d ==== nleft=%d\n",nread,nleft);
        rcv+=nread;
    }
    return nleft; 
}
//-------------------------------------------------------------------------------------------------------
int tcp_test(int sock) {
    int HEADER[10], rc;
    HEADER[0]=0;
    HEADER[1]=0xABCDEF00;
    HEADER[2]=0;
    HEADER[3]=0;
    HEADER[4]=0;
    HEADER[5]=0;
    HEADER[6]=0;
    
    while(1) {
	HEADER[1]=0xABCDEF00;
	fprintf(stderr,"tcp_test():: Send Req, Marker=0X%X \n",HEADER[1]); 
	while (send(sock, (char*) HEADER, sizeof(HEADER), 0) < 0) { 
	    TCP_FLAG=-2;
	    perror("tcp_test():send");
	    fprintf(stderr,"=> tcp_test():: Error send test HEADER: 0xABCDEF00\n");
	    while (tcp_event(NULL,0,0,0,0,0)<0) 
		fprintf(stderr,"=> tcp_test():1: Error Connect to Server ...\n");
	    fprintf(stderr,"=> tcp_test():1: Connected !!!  \n");
	}
	if ((rc=tcp_get3(sock,HEADER,sizeof(HEADER)))<0) 
	    fprintf(stderr,"tcp_test():: Error recv : rc=(%d)..\n",rc); 
	else {
	    fprintf(stderr,"tcp_test():: Recv rc=%d, Marker=0X%X \n",rc,HEADER[1]); 
	    if (HEADER[1]==0XABCDEF11) 
		return 0;
	}
	TCP_FLAG=-2;
	while (tcp_event(NULL,0,0,0,0,0)<0) 
	    fprintf(stderr,"=> tcp_test():2: Error Connect to Server ...\n");
	fprintf(stderr,"=> tcp_test():2: Connected !!!  \n");
    }
}

//-------------------------------------------------------------------------------------------------------
int tcp_event( unsigned int *DATA, int lenDATA,int n,int k, unsigned int evtHDR, unsigned int TriggerID )
{
   static char hostname[100];
   static int	sd;
   static struct sockaddr_in pin;
   struct hostent *hp;
   unsigned int *HEADER = DATA;
   unsigned int *EVT_HEADER=&DATA[10];
   int rc,nleft,nsent;
   static unsigned int time0,time1,time2;
   char* snd;
   int evtTrigID, evtModID, evtSize;
   time_t time_hold=0;
// 0=initial -1 -no connection -2 error send 
   if (PORT==0) {  //-- set HOST and PORT to default: localhost:20249
       strncpy(HOST,"localhost",128);
       PORT=20249;
       fprintf(stderr,"==> TCPclient:: set DEFAULT!! HOST=%s PORT=%d \n",HOST,PORT);
   }
   if(TCP_FLAG==0) { 
      TCP_FLAG=-2;       
	 time((time_t*)&time0);
       strcpy(hostname,HOST);
       fprintf(stderr,"TCPclient:: go find out about the desired host machine, TCP_FLAG: (0) => (-2/1) \n");
       	if ((hp = gethostbyname(hostname)) == 0) {
       		perror("gethostbyname");
		return -1;
	}
        printf( "IP=%u.%u.%u.%u \n",
                (unsigned char) hp->h_addr_list[0][0],	(unsigned char) hp->h_addr_list[0][1],
		(unsigned char) hp->h_addr_list[0][2], (unsigned char) hp->h_addr_list[0][3]);

	//-------- fill in the socket structure with host information
	memset(&pin, 0, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	pin.sin_port = htons(PORT);
	//-------- grab an Internet domain socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	    perror("socket");
	    return -1;
	}
	fprintf(stderr,"TCPclient:: try to connect to %s port=%d\n",hostname,PORT);
	//------- connect to PORT on HOST
	if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
	    perror("connect");
	    return -1;
	}
	TCP_FLAG=1;  //---  server OK, send DATA 
        current_sd=sd;
	fprintf(stderr,"TCPclient:: Connected to %s port=%d sd=%d !!!\n",hostname,PORT,sd);

   } else if (TCP_FLAG==-1) { //--- no server (error connect)

       time((time_t*)&time1);  //---- timer for retry

       fprintf(stderr,"TCPclient:: t1=%d  t0=%d t1-t0=%d \n",time1,time0,time1-time0);
       if ((time1-time0) > T_WAIT ) { time0=time1;
	   fprintf(stderr,"TCPclient:: %d RE-try to connect to %s, TCP_FLAG:: (-1) => (0)\n",time1-time0,hostname);
	   TCP_FLAG=0; 
       };
       return -1;
   } else if (TCP_FLAG==-2) { //--- server disappeared  (error send)
       fprintf(stderr,"TCPclient:: close socket... %s, TCP_FLAG:: (-2) => (-1) \n",hostname);
       close(sd); 
       time((time_t*)&time0);

       TCP_FLAG=-1; 
       return -1;
   };
   
   //--------------------------------------------------
   //------------   ENDIF TCP_FLAG  -------------------
   //--------------------------------------------------


   if (DATA==NULL) return 0;  //--- Connect and send INFO only !!!!

#if 0
   printf("tcp_event(): Send TCP HEADER:: RQ=0x%x Mark=0x%x Len=%d (%d)  TriggerID=0x%x(0x%x) ModID=%d RUN_Number=%d \n"
          ,HEADER[0], HEADER[1], HEADER[2],lenDATA, HEADER[4],TriggerID, HEADER[6], HEADER[7]);
   printf("tcp_event(): Send EVENT HEADER:: Trig=0x%x Size=%d ModID=%d type=0x%x Nrocs=%d \n"
          ,EVT_HEADER[1], EVT_HEADER[2], EVT_HEADER[0]>>24&0xff, EVT_HEADER[0]>>16&0xff, EVT_HEADER[0]&0xff);
#endif

   //-----  !!!!!!  -----  temporary overwrite TRIGGER_ID for DATA evts, and MOD_ID for TESTS !!!!!!!!!!!!!

   int EVT_Type=(DATA[0]>>24)&0xff;
   int DEV_Type=(DATA[0]>>16)&0xff;
   if (EVT_Type==0x2) {          //--  data events : as in  Event2005.h
//-- first TLU is ZERO !!!       
     // DATA[1]=TriggerID;    //-- if TLU number is ZERO => set fake Trigger number;
   } else {
     //printf("tcp_event():: EVT_Type=%d, BOR/EOR event Trig=%d siz=%d\n",EVT_Type,TriggerID,lenDATA); 
   }
   //------  !!!!!!!  -------  END TEST replace  ---------------

   if(DEBUG>5) printf("tcp_event():: try to send data   Trig=%d\n",TriggerID);

   nleft=lenDATA*4;
   snd=(char*)DATA;

   if (HOLD_TIME) { time(&time_hold);  printf("tcp_event():: hold %d sec \n",HOLD_TIME); while ((time(NULL)-time_hold)<=HOLD_TIME) usleep(1000);   HOLD_TIME=0;  }

   while(nleft>0){
       if(DEBUG>5) printf("tcp_event():: try to send = %d of %d Trig=%d\n",PACKSIZE,nleft,TriggerID);

       if (nleft<PACKSIZE) nsent=send(sd,snd, nleft, 0);
       else                nsent=send(sd,snd, PACKSIZE, 0);
       if (nsent <=0) { perror("tcp_event"); printf("tcp_event():: send error ret=%d \n",nsent); TCP_FLAG=-2; return -1;}
       nleft-=nsent;
       snd+=nsent;
       if(DEBUG>5) printf("tcp_event():: sent nsent=%d nleft=%d \n",nsent,nleft);
   }
   return nleft; 
}
//============================================================================

int tcp_event0( unsigned int *DATA, int lenDATA, int n, int k, unsigned int evtHDR, unsigned int TriggerID )
{
   static char hostname[100];
   static int	sd;
   static struct sockaddr_in pin;
   struct hostent *hp;
   int HEADER[10];
   int rc,nleft,nsent;
   static unsigned int time0,time1,time2;
   char* snd;
   int evtTrigID, evtModID, evtSize;
#ifdef JF_WIN
   WSADATA wsaData;
#endif
// 0=initial -1 -no connection -2 error send 
   if (PORT==0) {  //-- set HOST and PORT to default: localhost:20249
       strncpy(HOST,"localhost",128);
       PORT=20249;
       fprintf(stderr,"==> TCPclient:: set DEFAULT!! HOST=%s PORT=%d \n",HOST,PORT);
   }
   if(TCP_FLAG==0) { 

      TCP_FLAG=-2;       
#ifdef JF_WIN	
 	 if(WSAStartup(MAKEWORD(2,0), &wsaData) != 0)
         printf("WSA konnte nicht initialisiert werden. \n");
	 time0=timeGetTime()/1000;
#else
	 time((time_t*)&time0);
#endif
       strcpy(hostname,HOST);
       fprintf(stderr,"TCPclient:: go find out about the desired host machine, TCP_FLAG: (0) => (-2/1) \n");
       	if ((hp = gethostbyname(hostname)) == 0) {
       		perror("gethostbyname");
		return -1;
	}
        printf( "IP=%u.%u.%u.%u \n",
                (unsigned char) hp->h_addr_list[0][0],	(unsigned char) hp->h_addr_list[0][1],
		(unsigned char) hp->h_addr_list[0][2], (unsigned char) hp->h_addr_list[0][3]);

	//-------- fill in the socket structure with host information
	memset(&pin, 0, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	pin.sin_port = htons(PORT);
	//-------- grab an Internet domain socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	    perror("socket");
	    return -1;
	}
	fprintf(stderr,"TCPclient:: try to connect to %s port=%d\n",hostname,PORT);
	//------- connect to PORT on HOST
	if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
	    perror("connect");
	    return -1;
	}
	TCP_FLAG=1;  //---  server OK, send DATA 

	fprintf(stderr,"TCPclient:: Connected to %s port=%d !!!\n",hostname,PORT);

   } else if (TCP_FLAG==-1) { //--- no server (error connect)
#ifdef JF_WIN
       time1=timeGetTime()/1000;;
#else
       time((time_t*)&time1);  //---- timer for retry
#endif
       fprintf(stderr,"TCPclient:: t1=%d  t0=%d t1-t0=%d \n",time1,time0,time1-time0);
       if ((time1-time0) > T_WAIT ) { time0=time1;
	   fprintf(stderr,"TCPclient:: %d RE-try to connect to %s, TCP_FLAG:: (-1) => (0)\n",time1-time0,hostname);
	   TCP_FLAG=0; 
       };
       return -1;
   } else if (TCP_FLAG==-2) { //--- server disappeared  (error send)
       fprintf(stderr,"TCPclient:: close socket... %s, TCP_FLAG:: (-2) => (-1) \n",hostname);
#ifdef JF_WIN
       closesocket(sd);
       time0=timeGetTime()/1000;;
#else
       close(sd); 
       time((time_t*)&time0);
#endif
       TCP_FLAG=-1; 
       return -1;
   };
   
   //--------------------------------------------------
   //------------   ENDIF TCP_FLAG  -------------------
   //--------------------------------------------------


   if (DATA==NULL) return 0;  //--- Connect and send INFO only !!!!


   //-- if (DATA[1])  //-- commented for EUDET reset TLU !!!
       TriggerID=DATA[1];   //-- set TLU Trigger number, if not ZERO;

   if (TriggerID==0x55555555) {  //---  BOR event !!!
       fprintf(stderr,"tcp_event():: BOR Event !! Trig=%d, test connection !!\n",DATA[1]);
       tcp_test(sd);
   }
   HEADER[0]=0x5;
   HEADER[1]=0xAABBCCDD;
   HEADER[2]=lenDATA;
   HEADER[3]=evtHDR;
   HEADER[4]=TriggerID;
   HEADER[5]=n;
   HEADER[6]=k;

#ifdef JF_DEBUG
   printf("send HEADER size=%d\n",sizeof(HEADER));
#endif
   
   if(DEBUG>5) printf("tcp_event():: try to send header Trig=%d\n",DATA[1]);

   if (send(sd, (char*) HEADER, sizeof(HEADER), 0) == -1) {
       perror("tcp_event_hdr"); TCP_FLAG=-2;
       return -1; 
   }

#ifdef JF_DEBUG
   printf("send DATA lenDATA=%d bytes (%d words)\n",lenDATA*4,lenDATA);
#endif
   
/*
   evtTrigID=DATA[1];
   evtModID=(DATA[0]>>24)&0xf;
   evtSize=DATA[0]&0xfffff;
   printf("tcp_event():0:  Trg=%d(%d) Mod=%d(%d) siz=%d(%d), data[0]=%X\n",evtTrigID,TriggerID,evtModID,k,evtSize,lenDATA,DATA[0]);
*/
   //-----  !!!!!!  -----  temporary overwrite TRIGGER_ID for DATA evts, and MOD_ID for TESTS !!!!!!!!!!!!!

   int EVT_Type=(DATA[0]>>22)&0x3;
   int DEV_Type=(DATA[0]>>28)&0xf;
   if (EVT_Type==0x2) {          //--  data events : as in  Event2005.h
//-- first TLU is ZERO !!!       
       DATA[1]=TriggerID;    //-- if TLU number is ZERO => set fake Trigger number;
   } else {
       printf("tcp_event():: EVT_Type=%d, BOR/EOR event Trig=%d siz=%d\n",EVT_Type,TriggerID,lenDATA); 
   }
//--- fake ModID
/*
   DATA[0]&= ~(0xf << 24);
   unsigned int MODID=((k+8)&0xf)<<24;
   DATA[0] |= MODID;
*/
   evtTrigID=DATA[1];
   evtModID=(DATA[0]>>24)&0xf;
   evtSize=DATA[0]&0xfffff;

   //printf("tcp_event():1:  Trg=%d(%d) Mod=%d(%d) siz=%d(%d), data[0]=%X\n",evtTrigID,TriggerID,evtModID,k,evtSize,lenDATA,DATA[0]);
   //------  !!!!!!!  -------  END TEST replace  ---------------

   if(DEBUG>5) printf("tcp_event():: try to send data   Trig=%d\n",TriggerID);

   nleft=lenDATA*4;
   snd=(char*)DATA;
   while(nleft>0){
       if(DEBUG>5) printf("tcp_event():: try to send = %d of %d Trig=%d\n",PACKSIZE,nleft,TriggerID);
       nsent=send(sd,snd, nleft, 0);
       //printf("sent DATA\n");
       if (nsent <=0) { perror("tcp_event"); TCP_FLAG=-2; return -1;}
       nleft-=nsent;
       snd+=nsent;
   }
   /*
   while(nleft>0){
       if(DEBUG>5) printf("tcp_event():: try to send = %d of %d Trig=%d\n",PACKSIZE,nleft,TriggerID);
       if (nleft<PACKSIZE) nsent=send(sd,snd, nleft, 0);
       else                nsent=send(sd,snd, PACKSIZE, 0);
       //printf("sent DATA\n");
       if (nsent <=0) { perror("tcp_event"); TCP_FLAG=-2; return -1;}
       nleft-=nsent;
       snd+=nsent;
   }
   */
   return nleft; 
}

//---------------------------------------------------------------------------------------------------------------
//                      S E N D  
//---------------------------------------------------------------------------------------------------------------
int tcp_event_snd( unsigned int *DATA, int lenDATA,int n,int k, unsigned int evtHDR, unsigned int TriggerID )
{
   static char hostname[100];
   static int  sd;
   static struct sockaddr_in pin;
   struct hostent *hp;
   static int TCP_FLAG,HEADER[10];
   int nleft,nsent;
   static unsigned int time0,time1;
   char* snd;
#ifdef JF_WIN
   WSADATA wsaData;
#endif
// 0=initial -1 -no connection -2 error send 
   if (PORT==0) {  //-- set HOST and PORT to default: localhost:20249
       strncpy(HOST,"localhost",128);
       PORT=20249;
       printf("==> TCPclient:: set DEFAULT!! HOST=%s PORT=%d \n",HOST,PORT);
   }
   if(TCP_FLAG==0) { 

      TCP_FLAG=-2;       
#ifdef JF_WIN   
         if(WSAStartup(MAKEWORD(2,0), &wsaData) != 0)
         printf("WSA konnte nicht initialisiert werden. \n");
         time0=timeGetTime()/1000;
#else
         time((time_t*)&time0);
#endif
       strcpy(hostname,HOST);
       printf("TCPclient:: go find out about the desired host machine \n");
        if ((hp = gethostbyname(hostname)) == 0) {
                perror("gethostbyname");
                return -1;
        }
        printf( "IP=%u.%u.%u.%u \n",
                (unsigned char) hp->h_addr_list[0][0],  (unsigned char) hp->h_addr_list[0][1],
                (unsigned char) hp->h_addr_list[0][2], (unsigned char) hp->h_addr_list[0][3]);

        //-------- fill in the socket structure with host information
        memset(&pin, 0, sizeof(pin));
        pin.sin_family = AF_INET;
        pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
        pin.sin_port = htons(PORT);
        //-------- grab an Internet domain socket
        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            return -1;
        }

        printf("TCPclient:: try to connect to %s port=%d\n",hostname,PORT);
        //------- connect to PORT on HOST
        if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
            perror("connect");
            return -1;
        }
        printf("TCPclient:: CONNECTED to %s  port=%d  local_sock=%d port=%d\n",hostname,PORT,sd,ntohs(pin.sin_port));
        TCP_FLAG=1;  //---  server OK, send DATA 
   } else if (TCP_FLAG==-1) { //--- no server (error connect)
#ifdef JF_WIN
       time1=timeGetTime()/1000;;
#else
       time((time_t*)&time1);  //---- timer for retry
#endif
       printf("TCPclient:: t1=%d  t0=%d t1-t0=%d \n",time1,time0,time1-time0);
       if ((time1-time0) > T_WAIT ) { time0=time1;
           printf("TCPclient:: %d RE-try to connect to %s\n",time1-time0,hostname);
           TCP_FLAG=0; 
       };
       return -1;
   } else if (TCP_FLAG==-2) { //--- server disappeared  (error send)
       printf("TCPclient:: close socket... %s\n",hostname);
#ifdef JF_WIN
       closesocket(sd);
       time0=timeGetTime()/1000;;
#else
       close(sd); 
       time((time_t*)&time0);
#endif
       TCP_FLAG=-1; 
       return -1;
   };
   
   //--------------------------------------------------
   //------------   ENDIF TCP_FLAG  -------------------
   //--------------------------------------------------

   HEADER[0]=0x5;  //---  buffered for evb  
   HEADER[1]=0xAABBCCDD;
   HEADER[2]=lenDATA;
   HEADER[3]=evtHDR;
   HEADER[4]=TriggerID;
   HEADER[5]=n;
   HEADER[6]=k;
   HEADER[7]=k;

#ifdef JF_DEBUG
   printf("send HEADER size=%d\n",sizeof(HEADER));
#endif
   
   if (send(sd, (char*) HEADER, sizeof(HEADER), 0) == -1) {
       perror("send"); TCP_FLAG=-2;
       return -1; 
   }
#ifdef JF_DEBUG
   printf("send DATA lenDATA=%d bytes (%d words)  nmod=%d mod=%d trigger=%d\n",lenDATA*4,lenDATA,n,k, TriggerID);
#endif
   
   nleft=lenDATA*4;
   snd=(char*)DATA;
   while(nleft>0){
       if(DEBUG>3) printf("try to send = %d of %d\n",PACKSIZE,nleft);
       if (nleft<PACKSIZE) nsent=send(sd,snd, nleft, 0);
       else                nsent=send(sd,snd, PACKSIZE, 0);
       //printf("sent DATA\n");
       if (nsent <=0) { perror("send"); TCP_FLAG=-2; return -1;}
       nleft-=nsent;
       snd+=nsent;
   }
   return nleft; 
}


//---------------------------------------------------------------------------------------------------------------
//                      G E T 
//---------------------------------------------------------------------------------------------------------------
int tcp_event_get(char *HOST2, unsigned int *DATA, int *lenDATA, int *Nr_Modules ,int *ModuleID, unsigned int *TriggerID )
{
   static char hostname[100];
   static int   sd;
   static struct sockaddr_in pin;
   struct hostent *hp;
   static int TCP_FLAG,HEADER[10];
   int nleft,nread;
   static unsigned int time0,time1;
   char  *rcv;
   static unsigned int MARKER,REQUEST,REQUESTED;
   int evtTrigID, evtModID, evtSize;


#ifdef JF_WIN
   WSADATA wsaData;
#endif
// 0=initial -1 -no connection -2 error send 
   if(TCP_FLAG==0) { 

      TCP_FLAG=-2;       
#ifdef JF_WIN   
         if(WSAStartup(MAKEWORD(2,0), &wsaData) != 0)
         printf("WSA konnte nicht initialisiert werden. \n");
         time0=timeGetTime()/1000;
#else
         time((time_t*)&time0);
#endif
       strcpy(hostname,HOST2);
       printf("TCPclient:: go find out about the desired host machine \n");
        if ((hp = gethostbyname(hostname)) == 0) {
                perror("gethostbyname");
                return -1;
        }
        printf( "IP=%u.%u.%u.%u \n",
                (unsigned char) hp->h_addr_list[0][0],  (unsigned char) hp->h_addr_list[0][1],
                (unsigned char) hp->h_addr_list[0][2], (unsigned char) hp->h_addr_list[0][3]);

        //-------- fill in the socket structure with host information
        memset(&pin, 0, sizeof(pin));
        pin.sin_family = AF_INET;
        pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
        pin.sin_port = htons(PORT);
        //-------- grab an Internet domain socket
        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            return -1;
        }
        printf("TCPclient:: try to connect to %s port=%d\n",hostname,PORT);
        //------- connect to PORT on HOST
        if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
            perror("connect");
            return -1;
        }
        printf("TCPclient:: CONNECTED to %s  port=%d  local_sock=%d\n",hostname,PORT,sd);
        TCP_FLAG=1;  //---  server OK, send DATA 
   } else if (TCP_FLAG==-1) { //--- no server (error connect)
#ifdef JF_WIN
       time1=timeGetTime()/1000;;
#else
       time((time_t*)&time1);  //---- timer for retry
#endif
       printf("TCPclient:: t1=%d  t0=%d t1-t0=%d wait %d sec...\n",time1,time0,time1-time0,T_WAIT-(time1-time0));
       if ((time1-time0) > T_WAIT ) { time0=time1;
           printf("TCPclient:: %d RE-try to connect to %s\n",time1-time0,hostname);
           TCP_FLAG=0; 
       };
       return -1;
   } else if (TCP_FLAG==-2) { //--- server disappeared  (error send)
       printf("TCPclient:: close socket... %s\n",hostname);
#ifdef JF_WIN
       closesocket(sd);
       time0=timeGetTime()/1000;;
#else
       close(sd); 
       time((time_t*)&time0);
#endif
       REQUESTED=0;
       TCP_FLAG=-1; 
       return -1;
   };
   
   //--------------------------------------------------
   //------------   ENDIF TCP_FLAG  -------------------
   //--------------------------------------------------

   REQUEST=*Nr_Modules;
   //printf("REQUEST=%#X(%d)  REQUESTED=%d\n",REQUEST,REQUEST,REQUESTED);
   
   if (REQUESTED==0 && REQUEST>0x1000) {
       if (REQUEST==0x1002) {
           HEADER[0]=0x2;  //-- request for 1 event --
           REQUESTED=0;
       } else if (REQUEST==0x1003) {
           HEADER[0]=0x3;  //-- request continuous events --
           REQUESTED=1;
       } else if (REQUEST==0x1013) {
           HEADER[0]=0x13;  //-- request continuous exclusive events --
           REQUESTED=1;
       } else {
           HEADER[0]=0x2;  //-- request for 1 event default --
           REQUESTED=0;
       } 
       
       HEADER[1]=0xAABBCCDD;
       HEADER[2]=*lenDATA;
       HEADER[3]=0;
       HEADER[4]=0;
       HEADER[5]=*Nr_Modules;
       HEADER[6]=*ModuleID;
       
       //printf(" tcp_event_get(): ask DATA max lenDATA=%d\n",*lenDATA);
       
       if (send(sd, (char*) HEADER, sizeof(HEADER), 0) == -1) {
           perror("send"); TCP_FLAG=-2;
           return -1; 
       }
   }
   //-----------------------------------------------------

   //-- recv HEADER ---
   //printf(" tcp_event_get(): wait for HEADER \n");

   nread=0;        
   nleft=sizeof(HEADER);
   rcv=(char*)HEADER;
   while(nleft>0){
       nread=recv(sd,rcv,nleft, 0);
       if (nread <=0) {perror("recv_header"); TCP_FLAG=-2; return -1;}
       nleft-=nread;
       rcv+=nread;
   }
   MARKER= HEADER[1];
   *lenDATA=HEADER[2];
   *TriggerID=HEADER[4];
   *Nr_Modules=HEADER[5];
   *ModuleID=HEADER[6];
   
   //printf("HEADER OK=%#X, Nr_MOD=%d, ModID=%d, lenDATA=%d\n",MARKER,*Nr_Modules,*ModuleID,*lenDATA);
   
   //-- recv DATA ---
   //printf(" tcp_event_get(): wait for DATA \n");
   
   nleft=*lenDATA*4;
   rcv=(char*)DATA;
   while(nleft>0){
       nread=recv(sd,rcv, nleft, 0);
       if (nread <=0) { perror("recv_data"); TCP_FLAG=-2; return -1;}
       nleft-=nread;
       rcv+=nread;
       //printf("DATA recv=%d  nleft=%d\n",nread,nleft);
   }

   evtTrigID=DATA[1];
   evtModID=(DATA[0]>>24)&0xf;
   evtSize=DATA[0]&0xfffff;

   //printf("OK, TCP recv=%d  nleft=%d Trg=%d(%d) Mod=%d siz=%d\n",nread,nleft,evtTrigID,*TriggerID,evtModID,evtSize);

   return 0; 
}
//============================================================================





//============================================================================
#ifdef JF_WIN
static int  sig_pipe;
void fpipe(int m)  { sig_pipe=1;   printf("\n Signal PIPE received \n\r");}

int  main(int argc, char **argv)
{
#define BUFSIZE 128000
    static unsigned int BUFFER[BUFSIZE];
    int LENEVENT, rc;
    unsigned int nmod=5,hdr,i,itrg=1000;
    LENEVENT=BUFSIZE;
    signal(SIGPIPE,fpipe);
    
    while(1) { 
	itrg++;
	for (i=0;i<nmod;i++) {
            LENEVENT=100+itrg*10;
            hdr=(i & 0xF) << 24;
	    BUFFER[1]=itrg;
	    printf("-------------- NEW call to send %d %d %d DATA=%d\n",itrg,i,LENEVENT,BUFFER[1]);
	    rc=tcp_event(BUFFER,LENEVENT,nmod,i,hdr,itrg);
	    if (rc<0) printf(" ERROR send \n");
	}   
        sleep (3);
    }

	return 0;
}
#endif


#include <tcp_daq/tcp_event.h>
#include <sys/time.h>
//
//
//   g++ jana_main.cc jana_tcp.cc tcp_thread.cc -lm -o jana.exe
//  ./jana.exe -req=ex -cmd=recv
//  ./jana.exe -req=ex -cmd=send -host=gluonraid4:20249
//
//============================================================================
//                             M A I N 
//============================================================================
int STREQ(char*s1,const char*s2) { if (strncasecmp(s1,s2,strlen(s2))) return 0; else return 1; }
int  sig_pipe=0, sig_int=0, sig_hup=0, sig_alarm=0;

void fpipe(int m)  { sig_pipe=1;   printf("\n Signal PIPE received \n\r");}

void ctrl_c(int m) { sig_int=1;
   printf("\n CTRL-C pressed...\n"); 
   exit(0);
   //printf("\n CTRL-C pressed, cancel timer=%d\n",alarm(1)); 
}

int  main(int argc, char **argv)  {
#define BUFSIZE 128000
    static unsigned int BUFFER[BUFSIZE];
    int LENEVENT;
    int icc,icc_old=0,rc,Kmod, Nmod, udelay=0, prescale=1;
    unsigned int REQUEST=0, CMD=0;
    unsigned int nmod=1,hdr,i,itrg=0;
    struct timeval tv1;
    unsigned time_old=0,time_new,itrg_old=0;
    double rate=0;
    int modID=4;
//----------------------------- command line args ------------------------------
    const int LHOST=128;
    int CMD_PORT=32768, DATA_PORT=20249;
    char CMD_HOST[LHOST], DATA_HOST[LHOST], *substr1, *substr2;
    
    strncpy(CMD_HOST,"localhost",LHOST);     CMD_PORT=32768;
    strncpy(DATA_HOST,"localhost",LHOST);   DATA_PORT=20249;
 

    sig_int=0;
    sig_pipe=0;

    printf("N args=%d\n",argc);
    if (argc < 2) {
	printf("usage: %s -req[uest]=<all,one,ex[clusive]> [-cmd=<send,recv>] [-host=HOST[:PORT]] [-d[elay]=usec] -p[rescale=N]\n\n",argv[0]);
        //sleep(1);
	exit(1);
    } else {
	for (int ii=1;ii<argc;ii++) {
	    printf("arg(%d)=%s -> ",ii,argv[ii]);
	    //-------------------------------------------------------------------
	    if(STREQ(argv[ii],"-cmd_host"))   { 
		if ((substr1=strstr(argv[ii],"="))) {
		    printf("found /=/ |%s|\n",substr1);
		    if ((substr2=strstr(argv[ii],":"))) {
			int Lhost=(substr2-substr1)-1;
			printf("found /:/ |%s|, Lhost=%d\n",substr2,Lhost);
			strncpy(CMD_HOST,&substr1[1],Lhost); CMD_HOST[Lhost]=0;
			CMD_PORT=atoi(&substr2[1]);
		    } else {	
			strncpy(CMD_HOST,&substr1[1],LHOST);
		    }
		}
	    }
	    //-------------------------------------------------------------------
	    if(STREQ(argv[ii],"-data") || STREQ(argv[ii],"-host") )   { 
		if ((substr1=strstr(argv[ii],"="))) {
		    printf("found /=/ |%s|\n",substr1);
		    if ((substr2=strstr(argv[ii],":"))) {
			int Lhost=(substr2-substr1)-1;
			printf("found /:/ |%s|, Lhost=%d\n",substr2,Lhost);
			strncpy(DATA_HOST,&substr1[1],Lhost); DATA_HOST[Lhost]=0;
			DATA_PORT=atoi(&substr2[1]);
		    } else {	
			strncpy(DATA_HOST,&substr1[1],LHOST);
		    }
		}
	    }
	    //-------------------------------------------------------------------
	    if(STREQ(argv[ii],"-cmd"))   { 
		if ((substr1=strstr(argv[ii],"="))) {
		    printf("found /=/ |%s|\n",substr1);
		    if ((substr2=strstr(argv[ii],"send"))) {
			CMD=1;   //--- SEND --
		    } else if ((substr2=strstr(argv[ii],"recv"))) {
			CMD=0;  //--- RECV --  (default)
		    }
		}
	    }
	    //-------------------------------------------------------------------
	    if(STREQ(argv[ii],"-d"))   { //-- -delay
		if ((substr1=strstr(argv[ii],"="))) {
		    printf("found /=/ |%s|\n",substr1);
			udelay=atoi(&substr1[1]);
		}
	    }
	    //-------------------------------------------------------------------
	    if(STREQ(argv[ii],"-m"))   { //-- -module
		if ((substr1=strstr(argv[ii],"="))) {
		    printf("found /=/ |%s|\n",substr1);
			nmod=atoi(&substr1[1]);
		}
	    }
	    //-------------------------------------------------------------------
	    if(STREQ(argv[ii],"-id"))   { //-- -module ID
		if ((substr1=strstr(argv[ii],"="))) {
		    printf("found /=/ |%s|\n",substr1);
			modID=atoi(&substr1[1]);
		}
	    }
	    //-------------------------------------------------------------------
	    if(STREQ(argv[ii],"-p"))   { //-- Prescale printout
		if ((substr1=strstr(argv[ii],"="))) {
		    printf("found /=/ |%s|\n",substr1);
			prescale=atoi(&substr1[1]);
		}
	    }
	    //-------------------------------------------------------------------
	    if(STREQ(argv[ii],"-req"))   { 
		if ((substr1=strstr(argv[ii],"="))) {
		    printf("found /=/ |%s|\n",substr1);
		    if ((substr2=strstr(argv[ii],"all"))) {
			REQUEST=0x1003;   //--- req continuous events  
		    } else if ((substr2=strstr(argv[ii],"ex"))) {
			REQUEST=0x1013;  //--- req continuous exclusive events
		    } else if ((substr2=strstr(argv[ii],"one"))) {
			REQUEST=0x1002;  //---  //--- req one event
		    }
		}
	    }
	    //-------------------------------------------------------------------
	}   //-- end for(ii=
    }
    if (REQUEST==0) REQUEST=0x1002;      //--- default req one event

//    printf(" CMD_HOST=%s PORT=%d\n",CMD_HOST,CMD_PORT); 
    printf("DATA_HOST=%s PORT=%d\n",DATA_HOST,DATA_PORT); 
    printf("REQUEST=%#X \n",REQUEST); 
    printf("CMD=%d (0->recv, 1-> send) \n",CMD); 
    printf("Delay=%d \n",udelay); 
    printf("N_mod=%d \n",nmod); 
    printf("Prescale=%d \n",prescale); 
    sleep(3);
//-----------------------------------------------------------------------
    signal(SIGPIPE,fpipe);
    signal(SIGINT,ctrl_c);

    const unsigned int   BORE_TRIGGERID = 0x55555555;
    const unsigned int   EORE_TRIGGERID = 0xAAAAAAAA;
    int evt_type, temperature;
    int dev_type, ZSbit,SuccFrames;

    if (CMD==0) {   //--------------------   R E C V  (default) ---------------------------
      tcp_event_host(DATA_HOST, DATA_PORT);
	icc=0;
	unsigned char* pchar=(unsigned char*)BUFFER;
	while(!sig_int) { 
	    do {
		LENEVENT=BUFSIZE;
		Nmod=REQUEST;
//printf("==> RECV::  wait next evt ....\n"); 
		rc=tcp_event_get(DATA_HOST,BUFFER,&LENEVENT,&Nmod,&Kmod,&itrg);
//printf("==> RECV::  tcp_event_get() OK \n"); 
		int evtModID=(BUFFER[0]>>24)&0xf;
		int len2=BUFFER[0]&0xfffff;	
		evt_type = (BUFFER[0]>>22)&0x3;	
		dev_type = (BUFFER[0]>>28)&0xf;	
		ZSbit = (BUFFER[2]>>20)&0x1;
		SuccFrames = (BUFFER[2]>>16)&0xf;
		temperature = (BUFFER[2]>>22)&0x3ff;
		if (rc<0) { printf("==> RECV::  tcp_event_get() ERROR \n");  sleep(1); }
		else if(icc%prescale==0 || itrg==BORE_TRIGGERID || itrg==EORE_TRIGGERID || itrg<itrg_old 
			                || evt_type!=0x2 ) {
		    printf("==> RECV::(%d)  Mod=%d of %d,  Kmod=%d, ModID=%2d EvTyp=%d, DevTyp=%d, ZS=%d, T=%d, SuccFrm=%d Ndata=%d(%d) TrigID=%d(%d)"
			   ,icc,Kmod+1,Nmod, Kmod, evtModID,evt_type,dev_type,ZSbit,temperature,SuccFrames,LENEVENT,len2,itrg,BUFFER[1]);
		    if (Kmod==0) printf(" rate=%.1f \n",rate); else printf("\n");
		}
	    }  while (Kmod!=(Nmod-1));
//printf("==> RECV::  full evt OK \n"); 
	    if (evt_type!=0x2   && dev_type!=0x2 && dev_type!=0x3 && dev_type!=0x4)  continue;
	    if (dev_type==0x4 && ZSbit) { //-- V4 board
	      printf("==> RECV:BUFFER: ZSlen=%d \n",((LENEVENT-3)*4));
	      int hdr=12, cnt=0;
	      for (int i=hdr; i<(LENEVENT*4); i+=3) {  cnt++; 
		if (pchar[i]==0xFE) break; //-- 
		printf("%d(%d):x=%d,y=%d,adc=%d \n",cnt,i,pchar[i],pchar[i+1],pchar[i+2]);
	      }
	      printf("\n\n");
	    }
	    icc++;
	    if (icc%prescale==0) {
		gettimeofday(&tv1,NULL); time_new=(tv1.tv_sec*1000000+tv1.tv_usec);
		rate=(double)(icc-icc_old)/(double)(time_new-time_old)*1000000.;
		time_old=time_new;
		icc_old=icc;
		itrg_old=itrg;
	    }
	    if (udelay) usleep (udelay);
	}
    } else if (CMD==1) {  //--------------------   S E N D ---------------------------
	tcp_event_host(DATA_HOST, DATA_PORT);     

	unsigned int EVT_Type_BOR=(0x0&0x3)<<22;   //--  BOR=0x0
	unsigned int EVT_Type_EOR=(0x1&0x3)<<22;   //--  EOR=0x1
	unsigned int EVT_Type_DATA=(0x2&0x3)<<22;  //--  BOR=0x2
	unsigned int DEV_Type_GLUEX_128=(0x3&0xf)<<28;  //--  BOR=0x2
	

      itrg=0;
      int RocID;
      unsigned int MODID;
      //--------------------- send EOR for booking memory ------------------------
      int RunNumber=0;
      printf(" send EOR  \n"); 

      //LENEVENT=LEN+3;  //--  3 words cDAQ header !
      LENEVENT=3;  //--  only 3 words cDAQ header !

      int DC_NROC=0;

      BUFFER[0]=DC_NROC;  //-- DC_NROC if send to EVB, or  Run Number 16 bits only if send to DC !!!
      BUFFER[0] |= EVT_Type_EOR;              
      RocID=15; //
      MODID=(RocID&0xFF)<<24;   //-- ModID 8-bit IP based
      BUFFER[0] |= MODID;
      BUFFER[1]=EORE_TRIGGERID;
      BUFFER[2]=LENEVENT;  //--

      hdr=(1 & 0xF) << 24;
      
      for (i=0;i<nmod;i++) {
	BUFFER[0]&= ~(0xff << 24);             // --  clear ModID
	unsigned int MODIDx=((modID+i)&0xff)<<24;   //-- ModID=modID+i
	BUFFER[0] |= MODIDx;
	printf(" send EOR   RunNumber=%d Mod=%d \n",RunNumber,modID+i); 
	rc=tcp_event_snd(BUFFER,LENEVENT,1,RunNumber,hdr,EORE_TRIGGERID);  //-- send BOR 
	if (rc<0) { printf(" ERROR send \n"); sleep(1); }
      }

      sleep(1);

      //------------------- send BOR -----------------
      RunNumber=51515;
      printf(" send BOR   RunNumber=%d \n",RunNumber); 

      //LENEVENT=LEN+3;  //--  3 words cDAQ header !
      LENEVENT=3;  //--  only 3 words cDAQ header !

      BUFFER[0]=DC_NROC;  //--  DC_NROC if send to EVB, or  Run Number 16 bits only !!!
      BUFFER[0] |= EVT_Type_BOR;              
      RocID=15; //
      MODID=(RocID&0xFF)<<24;   //-- ModID 8-bit IP based
      BUFFER[0] |= MODID;
      BUFFER[1]=BORE_TRIGGERID;
      BUFFER[2]=LENEVENT;  //--

      hdr=(1 & 0xF) << 24;
      
      for (i=0;i<nmod;i++) {
	BUFFER[0]&= ~(0xff << 24);             // --  clear ModID
	unsigned int MODIDx=((modID+i)&0xff)<<24;   //-- ModID=modID+i
	BUFFER[0] |= MODIDx;
	printf(" send BOR   RunNumber=%d Mod=%d \n",RunNumber,modID+i); 
	rc=tcp_event_snd(BUFFER,LENEVENT,1,RunNumber,hdr,BORE_TRIGGERID);  //-- send BOR 
	if (rc<0) { printf(" ERROR send \n"); sleep(1); }
      }


      //-----------------  send DATA  ----------------
      printf(" send DATA  \n"); 

	while(!sig_int) { 
	    itrg++;
	    for (i=0;i<nmod;i++) {
	      //LENEVENT=4099;
	      LENEVENT=8195;
	      //LENEVENT=16387;
	      //LENEVENT=65539;
	      hdr=(i & 0xF) << 24;
	      
	      
	      BUFFER[0]=DC_NROC; // DC_NROC if send to EVB, or  
	      BUFFER[0] |= EVT_Type_DATA;
	      BUFFER[1]=itrg;
	      BUFFER[2]=LENEVENT;  //--
	      
	      BUFFER[0]&= ~(0xff << 24);             // --  clear ModID
	      unsigned int MODIDx=((modID+i)&0xff)<<24;   //-- ModID=modID+i
	      BUFFER[0] |= MODIDx;
	      
	      
	      unsigned int evtTrigID=BUFFER[1];
	      int evtModID=(BUFFER[0]>>24)&0xff;
	      int evtSize=BUFFER[2];
	      
	      if (itrg%prescale==0 || itrg<200) {
		if (i==0) {
		  gettimeofday(&tv1,NULL); time_new=(tv1.tv_sec*1000000+tv1.tv_usec);
		  rate=(double)(itrg-itrg_old)/(double)(time_new-time_old)*1000000.;
		  time_old=time_new;
		  itrg_old=itrg;
		}
		printf("==> SEND:: Trg=%d(%d,%d) Mod=%d(%d) siz=%d(%d), rate=%.1f  data=%.3f GB/s\n"
		       ,evtTrigID,itrg,BUFFER[1],evtModID,i,evtSize,LENEVENT,rate,LENEVENT*4*rate*nmod/1073741824.);
		
		
	      }
	      
	      rc=tcp_event_snd(BUFFER,LENEVENT,nmod,i,hdr,itrg);
	      if (rc<0) { printf(" ERROR send \n"); sleep(1); }
	    } 

	    /*
	    gettimeofday(&tv1,NULL); 
	    usleep (0);
	    gettimeofday(&tv2,NULL); 
            int ts = (tv2.tv_sec*1000000+tv2.tv_usec) - (tv1.tv_sec*1000000+tv1.tv_usec); 
	    printf("tsleep = %d \n",ts);
	    */
	    if (udelay) usleep (udelay); //--- udelay=0  bad idea !!!! performance !!!
	} //-- end while(!sig_int)
	

	//--------------------- send EOR ------------------------
      printf(" send EOR  \n"); 

      //LENEVENT=LEN+3;  //--  3 words cDAQ header !
      LENEVENT=3;  //--  only 3 words cDAQ header !

      BUFFER[0]=DC_NROC;  //--  DC_NROC if send to EVB, or  Run Number 16 bits only !!!
      BUFFER[0] |= EVT_Type_EOR;              
      RocID=15; //
      MODID=(RocID&0xFF)<<24;   //-- ModID 8-bit IP based
      BUFFER[0] |= MODID;
      BUFFER[1]=EORE_TRIGGERID;
      BUFFER[2]=LENEVENT;  //--

      hdr=(1 & 0xF) << 24;
      
      for (i=0;i<nmod;i++) {
	BUFFER[0]&= ~(0xff << 24);             // --  clear ModID
	unsigned int MODIDx=((modID+i)&0xff)<<24;   //-- ModID=modID+i
	BUFFER[0] |= MODIDx;
	printf(" send EOR   RunNumber=%d Mod=%d \n",RunNumber,modID+i); 
	rc=tcp_event_snd(BUFFER,LENEVENT,1,0,hdr,EORE_TRIGGERID);  //-- send EOR 
	if (rc<0) { printf(" ERROR send \n"); sleep(1); }
      }
    } //---    cmd==send  ----
    return 0;
}

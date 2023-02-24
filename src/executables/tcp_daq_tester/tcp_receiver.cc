#include <tcp_daq/tcp_event.h>
#include <tcp_daq/tcp_thread.h>
#include "tcp_receiver.h"
#include <sys/time.h>

//
//  g++ jana_input.cc jana_tcp.cc tcp_thread.cc -lpthread -lm -o jana_input.exe
//  ./jana_input.exe
//
//============================================================================
//                             M A I N 
//============================================================================
static void *run_client_thread(void *arg);

//int run_child(int CH_FLAG, int sd_current,int rem_port,FIFO_Buffer_list* fifo_buff_in, FIFO_Buffer_list* fifo_buff_out, FIFO* fifo_out, unsigned int Kclnt);
int run_child(int socket_fd, int rem_port, unsigned int Kclnt);

int STREQ(char *s1, const char *s2) { if (strncasecmp(s1, s2, strlen(s2))) return 0; else return 1; }

int sig_pipe = 0, sig_int = 0, sig_alarm = 0;

void on_signal_pipe(int m) {
    sig_pipe = 1;
    printf("\n Signal PIPE received \n\r");
}

void on_signal_ctrl_c(int m) {
    sig_int = 1;
    printf("\n CTRL-C pressed. Exiting...\n");
    exit(0);
}

pthread_t cl_thread[MAXCLNT];
unsigned int Client_mask[MAXCLNT];

static int N_clnt = 0;
pthread_mutex_t evb_mutex;

int rem_port, sock_main, sd_current;

#define DEBUG_RECV 0



//------------------------------------------------------------------------------


/*=====================================================================*/

static void *run_client_thread(void *arg) {
    auto *client = (struct client_t *) arg;

    int dc_flag = client->dc_flag;
    unsigned int client_id = client->client_number;

    int sd_current = client->sd_current;
    int rem_port = client->rem_port;

    printf("Started new client thread , DC_FLG=%d Kclnt=%d sd=%d \n", dc_flag, client_id, sd_current);

    run_child(sd_current, rem_port, client_id);

    printf("Thread return from Client No=%d close port=%d\n", client_id, sd_current);

    printf("Thread return from Client No=%d close port=%d\n", client_id, sd_current);

    close(sd_current);

    printf("EXIT child !!!!  clean client here k=%d Mask=%#x !!!!\n", client_id, Client_mask);

    Client_mask[client_id] = 0;
    N_clnt--;

    return 0;
}
/*=====================================================================*/
/*====================================================================*/
int run_child(int socket_fd, int rem_port, unsigned int Kclnt) {
    bool is_verbose = false;
    unsigned int Client_mask = 0;
    int pid, ppid;
    time_t t2;
    double tr1 = 0., tr2 = 0.;
    unsigned long int nev = 0, nev1 = 0, read_data_len = 0, rdata1 = 0;
    timeval tv1, tv[10], tm[10];
    time_t time_old = 0;
    long wdata = 0, wdata1 = 0;

    int PACKET[MAXDATA_DC + 10];
    int *BUFFER = &PACKET[10];
    //int BUFFER[MAXDATA];
    int header_data[10];
    //int *HEADER=PACKET;
    int icc = 0, READY3 = 0;
    int status;
    int LENEVENT, rc;
    int TriggerID = 0;
    int RunNum = 0;
    int ModuleID = 0, Nr_Modules, xxx;
    int evtModID, evtSize, eNtrig = 0, eNbuf[MAXMOD], eNuser = 0, eMuser = 0, eTimedOut = 0;
    unsigned int evtTrigID, oldTrigID = 0, old2TrigID = 0, errTrigID = 0, oldMod = 0;
    time_t time_now;
    unsigned int MARKER, REQUEST;
    float Rate = 0, Drate = 0, Buffer_Full;
    int N_Producers = 0;
    int ProducerID = 0;
    int first_enter = 1;
    int INFO_Triggernumber = 0;
    int TCP_FLAG = 1;
    int HOLD_FLAG = 0;
    char CMDDC[64];
    time_t time_hold = 0;
    //-- fifo --
    //FIFO *fifo=NULL;
//--------- for send shmem ------
    event_GLUEX copy_evt[MAXMOD];
    int lenEVENT[MAXMOD];
//-------------------------------
    struct EvtHeader EVENT_Group_Header;
    struct EvtHeader DC_Event_Header;

    pid = getpid();
    ppid = getppid();
    printf("=======> New process PID=%d  PPID=%d\n", pid, ppid);
    //event_ptr->N_Producers++;

    while (!sig_int) {

        if (TCP_FLAG == 0) {
            printf(" EXIT PORT=%d \n", socket_fd);
            return 1;
        }


        //--------------  PRODUCERS (ROCs/DCs)  -----------------
        if(is_verbose) {
            printf("=== Wait new HEADER pid=%d sd_current=%d Kclnt=%d ===\n", pid, socket_fd, Kclnt);
        }

        rc = tcp_get(socket_fd, header_data, sizeof(header_data));
        if (rc < 0) {
            printf("  tcp_get() get error: rc<0  (%d)......\n", rc);
            TCP_FLAG = 0;
            continue;
        }

        read_data_len += sizeof(header_data);

        REQUEST = header_data[0];
        MARKER = header_data[1];
        LENEVENT = header_data[2];
        xxx = header_data[3];
        TriggerID = header_data[4];
        Nr_Modules = header_data[5];
        ModuleID = header_data[6];
        RunNum = header_data[7];
        status = header_data[8];
        //printf("run_child recv::hdr ==> REQ=0x%X MARKER=0x%X LEN=%d TRG=0x%X Nmod=%d modID=%d \n",REQUEST,MARKER,LENEVENT,TriggerID,Nr_Modules,ModuleID);
        if (MARKER == 0xABCDEF00) {
            printf("!!!!! event-builder:: Test Connection MARKER=%X,  return 0xABCDEF11 \n", MARKER);
            //-- send HEADER --
            header_data[1] = 0xABCDEF11;
            header_data[2] = 0;
            header_data[3] = 0;
            header_data[4] = 0;
            header_data[5] = 0;
            header_data[6] = 0;
            if (tcp_send_th(socket_fd, header_data, sizeof(header_data))) {
                perror("send");
                TCP_FLAG = 0;
            }
            continue;
        }
        /*
          if (MARKER!=0xAABBCCDD) {
          //printf("!!!!! event-builder:: Error MARKER=%X\n",MARKER);
          printf("==> REQ=0x%X MARKER=0x%X LEN=%d TRG=0x%X Nmod=%d modID=%d run_DC_FLAG=%d \n",REQUEST,MARKER,LENEVENT,TriggerID,Nr_Modules,ModuleID,run_DC_FLAG);
          sleep(1);
          continue;
          }
        */
        if (TriggerID < 100) {
            printf(" >> RECV:0: REQUEST=0x%x, TriggerID=0x%x  evtSize=%d   (%d) ModID=%d  \n", REQUEST, header_data[4],
                   header_data[2], RunNum, header_data[6]);

        }




        //=================================================================================================================================
        //                                                               ---  recv BUFFERED ---
        //=================================================================================================================================
        if (REQUEST == 0x5) {  //---  recv BUFFERED ---;

            if (is_verbose) printf("get_from_client:: MARKER=%X\n", MARKER);

            if (LENEVENT > MAXDATA) {
                printf(" %c  \033[1m\033[31m  ERROR RECV:: event size > buffsize %d %lu trig=%d mod=%d \n", 7, LENEVENT,
                       MAXDATA, TriggerID, ModuleID);

                close(socket_fd);
                TCP_FLAG = 0;
                continue;
            }

            //---------------- get only header ; 3 words  !!!!!  --------
            // rc=tcp_get_th(sd_current,BUFFER,LENEVENT*4);
            rc = tcp_get(socket_fd, BUFFER, 3 * 4);
            if (rc < 0) {
                printf(" 2 get errorr rc<0 (%d)......\n", rc);
                TCP_FLAG = 0;
                continue;
            }
            else if (rc > 0) {
                printf("Need to get more data=%d \n", rc);
                TCP_FLAG = 0;
                continue;
            };

            read_data_len += LENEVENT * 4;

            evtTrigID = BUFFER[1];
            evtModID = (BUFFER[0] >> 24) & 0xff;
            evtSize = BUFFER[2];

            if (evtTrigID == EORE_TRIGGERID ||
                evtTrigID == BORE_TRIGGERID) {    //-------------       for memory book          -----------

                printf(" >> RECV:: TriggerID=0x%x (0x%x) evtSize=%d  got Run Number = %d ModID=%d \n", TriggerID,
                       evtTrigID, evtSize, RunNum, evtModID);
            }


            if (evtTrigID == BORE_TRIGGERID) {    //-------------         New run           -----------
                printf(" >> RECV:: BOR:: TriggerID=0x%x (0x%x)  \n", TriggerID, evtTrigID, RunNum);


                printf(" >> RECV:: TriggerID=0x%x try Lock mutex ModID=%d \n", TriggerID, evtModID);
                pthread_mutex_lock(
                        &evb_mutex); //------------------------- Looooooooooooooooooooooooooooooooock ----------------------


                printf(" >> RECV:: TriggerID=0x%x Unlock mutex ModID=%d\n", TriggerID, evtModID);
                pthread_mutex_unlock(
                        &evb_mutex);   //------------------------- Un-Looooooooooooooooooooooooooooooooock ----------------------
            }   //--------  if evtTrigID==BORE_TRIGGERID ------


            if (is_verbose || DEBUG_RECV > 0 || TriggerID < 0) {
                printf("+++>>> recv::hdr:: TrID=%d(0x%x) ModNr=%d len=%d; evt:: TrgID=%d, ModID=%d, len=%d (%d bytes)\n",
                       TriggerID, TriggerID, ModuleID, LENEVENT, evtTrigID, evtModID, evtSize, evtSize * 4);
                struct EvtHeader *hdr = (EvtHeader *) BUFFER;
                //printf("      header:: EvSize=%d EvType=%d ModID=%d DevType=%d Trigger=%d \n"
                //	   , hdr->EventSize, hdr->EventType, hdr->ModuleNo, hdr->DeviceType, hdr->Triggernumber);
            }

            if (evtSize != LENEVENT)
                printf("*****> %c ERROR RECV:: TrigID=%d, event size::  %d != %d (LENEVENT)\n", 7, evtTrigID, evtSize,
                       LENEVENT);
            /*
          else
          printf("*****> %c OK    RECV:: TrigID=%d, event size::  %d != %d (LENEVENT)\n",7,evtTrigID,evtSize,LENEVENT);
            */
            if (LENEVENT > MAXDATA) {
                printf("*****> %c ERROR RECV:: LENEVENT=%d  MAXDATA=%lu  bytes\n", 7, LENEVENT, MAXDATA);
            }

            gettimeofday(&tm[1], NULL);


            //     wait_buff:;
            //---- Copy recv Buffer to SHMEM -----
            if (DEBUG_RECV > 8) printf(" RECV: wait shmem\n");

            icc++;

            // fifo->AddEvent( BUFFER, LENEVENT);

            //unsigned int * wrptr = fifo->AddEvent3(LENEVENT);
            //---------------- get only header ; 3 words  !!!!!  --------
            rc = tcp_get(socket_fd, BUFFER,
                         (LENEVENT - 3) * 4);  // <<<<---------------- THIS IS DATA !!!! -----------------------
            //memcpy(wrptr,BUFFER,3*4);  //--- words to bytes
            //rc=tcp_get_th(sd_current,((int*)wrptr+3),(LENEVENT-3)*4);
            if (rc < 0) {
                printf(" 2 get errorr rc<0 (%d)......\n", rc);
                TCP_FLAG = 0;
                continue;
            }
            else if (rc > 0) {
                printf("Need to get more data=%d \n", rc);
                TCP_FLAG = 0;
                continue;
            };
            //unsigned int * wrptr2 = fifo->AddEvent3(0);

            if (evtTrigID == EORE_TRIGGERID) { printf("--> RECV:: added EOR event to FIFO !!\n"); }
            //fifo_buff_in->Show();

            //--------------------------------------------------  Equalizer -----------------------------------------------------------


            //-------------------------------------------------------------------------------------------------------------------------

            //if(sig_hup%2 || DEBUG_RECV>0 ) printf(" RECV: FOUND free fifo TriggerID=%d  \n",TriggerID);

            time(&t2);

            //---- end of SHMEM fill  -----

            if (DEBUG_RECV > 8) printf(" RECV: copy to shmem DONE !!!\n");
            nev++;
            gettimeofday(&tv1, NULL);
            tr2 = (double) tv1.tv_sec + (double) tv1.tv_usec * 1e-6;
            //printf("nev=%d nev1=%d t2=%d tr2=%f(%d),  tr1=%f,  tr2-tr1=%f\n"
            //	 ,nev,nev1,t2,tr2,tv1.tv_sec,tr1,tr2-tr1);
            if ((tr2 - tr1) > 5. || nev < 10) {
                if (nev1 != nev) {
                    Rate = (nev - nev1) / (tr2 - tr1);
                    Drate = (read_data_len - rdata1) / (tr2 - tr1) / 1000000.;
                    //Buffer_Full=(float)event_ptr->N_buf[ProducerID]/(float)(MAXEVT)*100.;
                    //double ff_lev = fifo->TCPBuffer->fifo->full_level;
                    printf("+RECV_%d_> Event=%ld LEN=%d Rate=%.1f Hz, Data=%.3f MB/s \n", rem_port, nev, LENEVENT, Rate,
                           Drate);
                    //	  printf("rdata= %ld %ld %ld --  time = %f %.1f %.1f \n",rdata,rdata1,rdata-rdata1,tr2,tr1,tr2-tr1 );
                    //printf("sdata= %ld %ld %ld --  time = %f %.1f %.1f nev=(%ld %ld %ld )\n",rdata,rdata1,rdata-rdata1,tr2,tr1,tr2-tr1,nev,nev1,nev-nev1 );
                    nev1 = nev;
                    read_data_len = 0;
                    rdata1 = read_data_len;
                    tr1 = tr2;
                } else {
                    //printf("RECV++> Event=%d, STAU:  wait next 5 sec...\n",nev);
                }
            }
        }  //--- end if  of send/recv req=0x5 -----


    }  //---   end of while(!sigint)

    return 0; // Not sure what this is intended for. DL 2/24/2023
}




int main(int argc, char **argv) {

    unsigned int Client_mask = 0, Nclnt = 0, Kclnt = 0;
    //static char path[]="./prof/";
    int Mod_tot, N_BOR_tot = -1;
    pid_t pid;
    char *substr1, *substr2;
    struct network_host_t cmd_host;
    struct network_host_t data_host;
    struct network_host_t seb_host;


    /*-----------  default value for hosts/ports ---------------------------*/
    strncpy(cmd_host.name, "localhost", HOST_NAME_LEN);
    cmd_host.port = 32768;

    strncpy(data_host.name, "localhost", HOST_NAME_LEN);
    data_host.port = 20249;

    Mod_tot = 1; //---  !!!!  check MAXMOD  !!!;

    printf(" CMD_HOST=%s PORT=%d\n", cmd_host.name, cmd_host.port);
    printf("DATA_HOST=%s PORT=%d\n", data_host.name, data_host.port);



    // Set signals
    signal(SIGINT, on_signal_ctrl_c);
    //signal(SIGALRM,falarm_BOR);
    //signal(SIGHUP,hungup);
    signal(SIGPIPE, on_signal_pipe);

    /*--------------------------------------*/
    printf("Process ID of the calling process =%d\n", getpid());

    pthread_mutex_init(&evb_mutex, NULL);

    //================================================================
    while(true) {
        printf("Binding socket...\n", getpid());
        sock_main = tcp_open_local_th(data_host.port);

        if (sock_main < 0) {
            // Could not open socket, sleep before retrying
            sleep(2);
        }
        else
        {
            break;  // Success, continue with code
        }
    }
    //============================================================
    //           wait for incoming  connections loop
    //============================================================

    printf("Waiting for clients... \n");
    int cl_DC_FLAG;

    while (!sig_int) {
        printf(" while_loop:: PORT=%d ", data_host.port);

        char host_name[LINFO];

        rem_port = tcp_listen3(sock_main, host_name, LINFO, &sd_current);
        printf(" return from listen3 , blin, ADD NEW Client sem wait ... remport =%d \n", rem_port);
        cl_DC_FLAG = 0;


        if (N_clnt >= MAXCLNT) {
            printf("****>  ERROR add Client_%d MAX Number of clients=%d", N_clnt + 1, MAXCLNT);
            continue;
        }


        printf(" ADD NEW Client DONE N_clnt=%d Kclnt=%d Mask=%#x !!! cl_DC_FLAG=%d\n", Nclnt, Kclnt, Client_mask,
               cl_DC_FLAG);
        //---  end add new client -------
        //pid=fork2();
        //--------------------------------------------------------------
        struct client_t sclnt;
        sclnt.dc_flag = cl_DC_FLAG;
        sclnt.client_number = Kclnt;
        sclnt.sd_current = sd_current;
        sclnt.rem_port = rem_port;
        //sclnt.fifo_out=fifo_out;
        //sclnt.fifo_buff_out=fifo_buff_out;
        //sclnt.fifo_buff_in=fifo_buff_in;
        int ret = pthread_create(&cl_thread[Kclnt], 0, run_client_thread, &sclnt);
        if (ret) {
            perror("pthread_create");
        }
        char th_name[64];
        sprintf(th_name, "cl_thread_%d", Kclnt);
        ret = pthread_setname_np(cl_thread[Kclnt], th_name);
        printf("\n detach run_child thread Kclnt=%d\n", Kclnt);
        ret = pthread_detach(cl_thread[Kclnt]);
        if (ret != 0) fprintf(stderr, "detach %d failed %d\n", Kclnt, ret);

        //----------------------------------------------------

        printf(" Create child \n");
        //close(sd_current);
    }  //--- end While(1) Loop

    /*-----------------------------------------------------------*/
    printf(" Close TCP ports \n");
    close(sock_main);
    //shmdt ((char*)event_ptr);  fflush(stdout);
    exit(0);
}
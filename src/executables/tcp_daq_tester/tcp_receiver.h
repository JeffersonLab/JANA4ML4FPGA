#include <string>
#include <list>
#include <iostream>
#include <sstream>

using namespace std;
const int HOST_NAME_LEN=128;

struct network_host_t {
    int port;
    char name[HOST_NAME_LEN];
    int mod_id;
};

#define MAXCLNT 32  //--  used 32 bit word   !!!!
#define MAXMOD  15L      //-- max 15 modules/crates
#define MAXDATA_DC  300000L  //-- in words; 
#define MAXDATA 65536L   //-- in words;  400kB max (RAW CDC) => 100000L


struct client_t {
  int dc_flag;           //-- DC flag
  int client_number;     //-- client number
  int sd_current;        //-- tcp port number
  int rem_port;          //-- rem port?
  //FIFO *fifo_out;
  //FIFO_Buffer_list* fifo_buff_out;
  //FIFO_Buffer_list *fifo_buff_in;
};
#define   BORE_TRIGGERID  0x55555555
#define   INFO_TRIGGERID  0x5555AAAA
#define   EPICS_TRIGGERID 0xAAAA5555
#define   EORE_TRIGGERID  0xAAAAAAAA

struct EvtHeader {   //--- 3 words ---
    unsigned int    Nrocs: 8;
    unsigned int    Reserved: 8;
    unsigned int    EventType: 8;
    unsigned int    ModuleNo: 8;
    unsigned int    Triggernumber; 
    unsigned int    EventSize;
};

//----------------------------------
struct BOR_HEADER
{ 
    struct EvtHeader Group;
    struct EvtHeader Module[MAXMOD];
};
struct INFO_HEADER
{ 
    struct EvtHeader Group;
    struct EvtHeader Module[MAXMOD];   //-- is not used --
};
//----------------------------------
typedef struct
{ 
    unsigned int HEADER;
    unsigned int Trigger; 
    unsigned int EventSize;
    unsigned int DATA[MAXDATA];
}
event_GLUEX;
//----------------------------------
typedef struct 
{
    int Status; //- free=0/busy=1/ready=2/
    unsigned int Trig_ID;
    unsigned int Client_bits;
    int M_users;
    int N_Trig;
    time_t Time_mark;
    time_t TTL;   //-- 8 bytes
//    unsigned int Size32;
    struct EvtHeader Group;
    int N_Mod;
    int Mod_ID[MAXMOD];
    event_GLUEX  gluex_evt[MAXMOD];
} 
event_slot;


//----------------------------------
typedef struct 
{
#define LINFO 128
    int   Port;
    time_t Time;
    int   Kclnt;  //-- for cross check 
    int   Request;
    int   N_events;
    float Rate;
    char  Host[LINFO];
    char  Type[LINFO];
    float Fifo_in;
    float Fifo_out;
    int   ModID;
} 
Client_Info_struc;
//----------------------------------

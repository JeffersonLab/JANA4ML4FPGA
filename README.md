A4ML4FPGA
EIC R&amp;D supported project developing ML on FPGA for streaming readout systems


## evio data

All DATA is here:

```
/gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_*.evio
```
Physics runs:
-------------

```
===>  2 crates , 3 detectors : CAL/FA250, GEMTRD/FA125 , GEM/SRS

CODA::  Run_2531 GEMTRD:ok; CAL:ok; SRS:del=0x41; ROSYBC=0x90 3bin; 10APV;OK; TRDpos=100; 5.1M ev  *PHYS* !!!!
cDAQ::  Run_2543 GEMTRD:ok; CAL:ok; SRS:del=0x41; ROSYBC=0x90 3bin; 10APV;OK; TRDpos=100; 1.1M ev  *PHYS* !!!!


===> 1 crate ; 2 detectors : GEMTRD/FA125 ; GEM/SRS

CODA::  Run_2548 (CODA) GEMTRD:ok; CAL:No; SRS:del=0x40; ROSYBC=0x70 9bin; 10APV;OK; TRDpos=155; 1.5M ev  *PHYS* !!!!
cDAQ::  Run_2567 (cDAQ) GEMTRD:ok; CAL:No; SRS:del=0x40; ROSYBC=0x70 9bin; 10APV;OK; TRDpos=155; 3.2M ev  *PHYS* !!!!

===> new files Mode8 (RAW) and Mode5 (short)


cDAQ / rawmode (8) ; trd_ti_fp.conf ; 300 nA; 650-700Hz ; 16 MB/s
14:09 Run_2633 (cDAQ) GEMTRD:ok; CAL:on; SRS:del=0x41; ROSYBC=0x70 3bin; 10APV;OK; TRDpos=150; Mode8; 250K evt;


cDAQ / Mode5; thr300 / trd_ti_fp_m5.conf ; 300 nA; 650-700Hz ; 7.5 MB/s
14:48 Run_2635 (cDAQ) GEMTRD:ok; CAL:on; SRS:del=0x41; ROSYBC=0x70 3bin; 10APV;OK; TRDpos=50;  Mode5; 250 K evt;
```

Test setup configuration:

- rocFMWPC1 is a TI master (no TSG), with single fa250 board, the last 3 channels contain data from the calorimeter.
- rocTRD1 is a slave with 4 fa125 boards reading GEMTRD (bank 16) also provides SRS/GEM data - bank 17.


Example of reading evio file: 

```bash
/mnt/c/eic/data/2023-02_ml4fpga_trd_data/hd_rawdata_002539_000.evio
```


## TCP test sender and receiver

There are two test executables built with the project that could be used for testing

- tcp_sender - can send dummy evio events through tcp
- tcp_receiver - can receive them

To make test loop: sender sends data to receiver - drinks coffee (or what it does): 

```bash
# In one terminal/process
tcp_receiver

# In another terminal/process
tcp_sender -req=ex -cmd=send -host=localhost:20249
```


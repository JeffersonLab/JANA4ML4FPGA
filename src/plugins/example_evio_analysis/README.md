```bash
-Pplugins=log,root_output,CDAQfile,example_evio_analysis
-Pjana:debug_plugin_loading=1
-PSingleEvioEventFileSource:LogLevel=trace
-Pexample_evio_analysis:LogLevel=trace
-Pjana:timeout=0
-Pjana:nevents=10
-Pnthreads=1
-Phistsfile=/home/romanov/eic/JANA4ML4FPGA/cmake-build-debug/test.root
/mnt/work/data/2023-03-03-trd-data/hd_rawdata_002633_000.evio
```


```bash
jana4ml4fpga
-Pplugins=log,root_output,flat_tree,CDAQfile,gemrecon
-Pjana:debug_plugin_loading=1
-PCDaqfile:srs_nsamples=20
-PCDAQEVIOFileSource:LogLevel=trace
-Pexample_evio_analysis:LogLevel=trace
-Pgemrecon:LogLevel=trace
-Pgemrecon:mapping=/home/romanov/eic/JANA4ML4FPGA/src/plugins/gemrecon/db/2019_mapping_GEMTRK.cfg
-Pgemrecon:config=/home/romanov/eic/JANA4ML4FPGA/src/plugins/gemrecon/db/Config.cfg
-Pjana:timeout=0
-Pjana:nevents=50
-Pnthreads=1
-Phistsfile=/home/romanov/eic/JANA4ML4FPGA/cmake-build-debug/test.root
/mnt/work/data/gemtrd/hd_rawdata_001068_000.evio
```


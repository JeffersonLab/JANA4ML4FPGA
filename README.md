A4ML4FPGA
EIC R&amp;D supported project developing ML on FPGA for streaming readout systems


## evio reader

The plugin, that can decode evio data and read *.evio files is `src/jana/EVIOParser`


```
# Here are the files in cDAQ format, you can try to parse:
/gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002539_*.evio

# just for cross checking here are the standard CODA files:
/gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002532_*.evio
```

Test setup configuration:

- rocFMWPC1 is a TI master (no TSG), with single fa250 board, the last 3 channels contain data from the calorimeter.
- rocTRD1 is a slave with 4 fa125 boards reading GEMTRD (bank 16) also provides SRS/GEM data - bank 17.


Example of reading evio file: 

```bash
/mnt/c/eic/data/2023-02_ml4fpga_trd_data/hd_rawdata_002539_000.evio
```

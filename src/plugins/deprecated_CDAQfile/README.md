# JANA CDAQfile plugin

Create a source plugin by pulling codes from `/rawdataparser`.
The plugin opens a `*.evio` file and print its file summary.

### Build

```bash
[hdtrdops@gluon200 EVIOParser]$ pwd
/gluonfs1/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/

## build plugin, install to $ROOT/install/plugins
cmake -S . -B build -DCMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT=1
cmake --build build --target install -j16


jana -Pplugins=CDAQfile -Pevent_source_type="CDAQEVIOFileSource" /gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002539_002.evio
```

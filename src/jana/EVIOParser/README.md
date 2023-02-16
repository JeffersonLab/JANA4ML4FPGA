# JANA EVIO event source plugin

Create an EVIO source plugin by pulling codes from `../rawdataparser`.
Now `JEventSourceEVIOSource::Open()` opens a `*.evio` file and print its file summary.

### Build

```bash
[hdtrdops@gluon200 EVIOParser]$ pwd
/gluonfs1/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/

## build plugin
cmake3 -S . -B build -DCMAKE_CXX_STANDARD=17
cmake3 --build build --target install -j16

## the plugin will be installed at $JANA_HOME/plugins
## Call the plugin
jana -Pplugins=janaEVIOSource -Pevent_source_type="JEventSourceEVIOSource" /gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002539_007.evio
```
### TODO
- [x] Implement `JEventSourceEVIOSource::GetEvent()` method.
- [ ] Other methods from legacy codes.

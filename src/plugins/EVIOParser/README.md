# JANA EVIO event source plugin

Create an EVIO source plugin by pulling codes from `../rawdataparser`.
Now `JEventSourceEVIOSource::Open()` opens a `*.evio` file and print its file summary.

### Build

```bash
[hdtrdops@gluon200 EVIOParser]$ pwd
/gluonfs1/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/plugins/EVIOParser

[hdtrdops@gluon200 EVIOParser]$ env | grep JANA
PATH=/u/apps/root/6.26.10/root-6.26.10-gcc9.3.0/bin:/home/hdtrdops/GemTrd_2023/JANA2/install/bin:/home/hdtrdops/GemTrd_2023/JANA2/install/bin:/apps/python3/3.9.7/bin:/apps/gcc/9.3.0/bin:/usr/lib64/qt-3.3/bin:/home/hdtrdops/perl5/bin:/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/opt/ibutils/bin:/opt/puppetlabs/bin
PWD=/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/plugins/EVIOParser
PYTHONPATH=/home/hdtrdops/GemTrd_2023/JANA2/install/python:/u/apps/root/6.26.10/root-6.26.10-gcc9.3.0/lib:/home/hdtrdops/GemTrd_2023/JANA2/install/python:
JANA_HOME=/home/hdtrdops/GemTrd_2023/JANA2/install
JANA2_MYSQL_CONFIG=/usr/bin/mysql_config

## build plugin
cmake3 -S . -B build -DCMAKE_CXX_STANDARD=17
cmake3 --build build --target install -j16

## the plugin will be installed at $JANA_HOME/plugins
## Call the plugin
plugins -Pplugins=janaEVIOSource -Pevent_source_type="JEventSourceEVIOSource" /gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002539_007.evio
```
### TODO
- [ ] Implement `JEventSourceEVIOSource::GetEvent()` method.
- [ ] Other methods from legacy codes.

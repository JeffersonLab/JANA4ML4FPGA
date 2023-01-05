#!/bin/bash

echo "./event-builder-er.exe -cmd=gluon25:32767 -f=/gluondaqfs/hdops/CDAQ/daq_dev_v0.31/TEST -raid=2 -m=4"

for id  in  4 5 6 7 
do
  xterm -e "./TCPclient.exe -cmd=send -p=10000 -d=2000000 -host=gluonraid4 -m=1 -id=${id}" &
done

#!/bin/bash

# split -l 31 file_fermi_3bin.list file_fermi_3bin.
# split -l 30 file_fermi_6bin.list file_fermi_6bin.

LISTS=${1-file_list}
SRSBIN=${2-6}

source setup_env.sh
pwd

while read lst  ; do

    echo " use list = $lst "

#    while true ; do
#        proc_list=`ps -ef | grep -v grep | grep "mss/halld" | awk '{printf("%s %d %d \n",$1,$2,$3) }'`
#        echo "PROC_LIST=$proc_list"
#        NJOBS=`ps -ef | grep java | grep jasmine | grep -v grep | grep jget`
#        if [[ x"$proc_list" == "x" ]] ; then
#            break
#        else 
#            sleep 10
#        fi
#    done

    echo " take next list $lst "

#    sleep 5

    echo "sh run_evio.sh $lst  0 SRS ${SRSBIN} "
   nohup  ./run_evio.sh $lst 0 SRS ${SRSBIN}  > LOG/$lst.log &

#    sleep 5

done < ${LISTS}

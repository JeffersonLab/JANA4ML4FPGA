#!/bin/bash

RUN=${1-help}
MAXEVT=${2-0}
MODE=${3-ADC}
SRSBIN=${4-3}


if [[ $RUN == "help" ]] ; then
    echo -e "\n Usage:  $0 <run_num> [max events] [SRS,ADC] \n"
    exit 0
fi

SRS_MAPPING=
if   (( 3000 < $RUN && $RUN <= 3139 )) ; then
    SRS_MAPPING="db/2023_mapping_fermilab1.cfg"
elif   (( 3140 < $RUN && $RUN <= 3147 )) ; then
    SRS_MAPPING="db/2023_mapping_fermilab3.cfg"
elif (( 3147 < $RUN && $RUN <= 4000 )) ; then
    SRS_MAPPING="db/2023_mapping_fermilab3.cfg"
fi
echo "SRS_MAPPING = $SRS_MAPPING"


#exit 0

RUNNUM=$(printf '%06d' ${RUN} ) 

FILELIST="`/bin/ls DATA/hd_rawdata_${RUNNUM}_*.evio `"

echo "FILELIST = $FILELIST "
echo "RUN = $RUN "
echo "SRSBIN = $SRSBIN "
if [[ $MAXEVT == 0 ]] ; then
    echo "MAXEVT = ALL "
else
    echo "MAXEVT = $MAXEVT "
fi


if [[ $MODE  == "ADC" ]] ; then

    echo " MODE = ADC "
    sleep 1

    jana4ml4fpga -Pplugins=log,root_output,flat_tree,CDAQfile   -Pnthreads=1 -Pjana:nevents=${MAXEVT} \
	-Phistsfile=ROOT/Run_${RUNNUM}.root  $FILELIST | grep -v flat_tree

elif [[ $MODE ==  "DUMP" ]] ; then

    echo " MODE = DUMP "
    sleep 1

    jana4ml4fpga -Pplugins=log,root_output,flat_tree,CDAQfile -Pnthreads=1 -Pjana:nevents=${MAXEVT} \
	-PEVIO:output_file=${RUNNUM}.evio -Phistsfile=ROOT/Run_${RUNNUM}_${FILENUM}.root $FILELIST

else
    echo " MODE = SRS "
    sleep 1


    jana4ml4fpga -Pplugins=CDAQfile,flat_tree,root_output,gemrecon -Pjana:nevents=${MAXEVT} \
	-Pdaq:srs_window_raw:ntsamples=${SRSBIN} \
        -Pjana:debug_plugin_loading=1\
	-Pevio:LogLevel=trace -Pgemrecon:LogLevel=trace \
        -Pgemrecon:LogLevel=info\
        -Pgemrecon:ClusterF:LogLevel=info\
	-Pgemrecon:mapping=${SRS_MAPPING} \
	-Phistsfile=ROOT/Run_${RUNNUM}.root  $FILELIST
fi



#jana4ml4fpga \
# -Pplugins=CDAQfile,flat_tree,root_output,gemrecon\
# -Pjana:nevents=5000\
# -Pjana:debug_plugin_loading=1\
# -Pdaq:srs_window_raw:ntsamples=3\
# -Pevio:LogLevel=trace\
# -Pgemrecon:LogLevel=info\
# -Pgemrecon:ClusterF:LogLevel=info\
# -Pgemrecon:mapping=/gluonfs1/home/hdtrdops/soft_ml4fpga/jana4ml4fpga/jana4ml4fpga-main/src/plugins/gemrecon/db/2023_mapping_fermilab.cfg\
# -Phistsfile=/gluonfs1/home/hdtrdops/soft_ml4fpga/003103_hists.root\


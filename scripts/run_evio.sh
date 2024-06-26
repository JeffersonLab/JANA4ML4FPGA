#!/bin/bash
# What this file does:
read -r -d '' HELP_TEXT << EOH
This Bash script processes raw data using jana4ml4fpga command.
It uses ./DATA folder to read .evio file and ./ROOT to save output.
Parameters:
- 'RUN'    - Identifies a specific run number to be processed.
- 'MAXEVT' - Maximum events to process. If set to 0, all events are processed. Defaults to 0.
- 'MODE'   - Can be "ADC", "DUMP", or "SRS". Defaults to "ADC".
- 'SRSBIN' - Used when 'MODE' is "SRS". Number of SRS time bins . Defaults to 3.
Usage:
run_evio.sh [run_num] [max events] [mode] [srsbin] [file_num]
EOH

RUN=${1-help}
MAXEVT=${2-0}
MODE=${3-ADC}
SRSBIN=${4-3}
FILE=$5

if [[ $RUN == "help" ]] || [[ $RUN == "--help" ]] ; then
    echo -e "\n$HELP_TEXT\n"
    exit 0
fi

SRS_MAPPING=
if ((3000 < $RUN && $RUN <= 3156)) ; then 
    SRS_MAPPING="db/2023_fermi_SRSmap0.cfg"
elif (( 3156 < $RUN && $RUN <= 3261)) ; then 
    SRS_MAPPING="db/2023_fermi_SRSmap1.cfg"
elif ((3261 < $RUN && $RUN <=3299)) ; then
    SRS_MAPPING="db/2023_fermi_SRSmap2.cfg"
elif ((4000 < $RUN && $RUN <=4999)) ; then
    SRS_MAPPING="db/2023_mapping_HDGEM.cfg"
fi

echo "SRS_MAPPING = $SRS_MAPPING"


#exit 0

RUNNUM=$(printf '%06d' ${RUN} ) 

if [[ x$FILE == "x" ]] ; then 
    echo " All Files "
    FILELIST="`/bin/ls DATA/hd_rawdata_${RUNNUM}_*.evio `"
else 
    FILENUM=$(printf '%03d' ${FILE} )
    FILELIST="`/bin/ls DATA/hd_rawdata_${RUNNUM}_${FILENUM}.evio `"
    echo " Process file = $FILELIST "
fi
sleep 1

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
    set -x
    jana4ml4fpga -Pplugins=log,root_output,flat_tree,CDAQfile   -Pnthreads=1 -Pjana:nevents=${MAXEVT} \
    -Phistsfile=ROOT/Run_${RUNNUM}.root  $FILELIST | grep -v flat_tree
    set +x

elif [[ $MODE ==  "DUMP" ]] ; then

    echo " MODE = DUMP "
    sleep 1
    set -x
    jana4ml4fpga -Pplugins=log,root_output,flat_tree,CDAQfile -Pnthreads=1 -Pjana:nevents=${MAXEVT} \
    -PEVIO:output_file=${RUNNUM}.evio -Phistsfile=ROOT/Run_${RUNNUM}_${FILENUM}.root $FILELIST
    set +x

else
    echo " MODE = SRS "
    sleep 1

    set -x
    jana4ml4fpga -Pplugins=CDAQfile,flat_tree,root_output,gemrecon,dqm \
    -Pjana:nevents=${MAXEVT} \
    -Pjana:timeout=0 \
    -Pdaq:srs_window_raw:ntsamples=${SRSBIN} \
    -Pjana:debug_plugin_loading=1 \
    -Pevio:LogLevel=trace \
    -Pgemrecon:LogLevel=info \
    -Pgemrecon:LogLevel=info \
    -Pgemrecon:ClusterF:LogLevel=info \
    -Pgemrecon:mapping=${SRS_MAPPING} \
    -Phistsfile=ROOT/Run_${RUNNUM}.root  $FILELIST
    set +x
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


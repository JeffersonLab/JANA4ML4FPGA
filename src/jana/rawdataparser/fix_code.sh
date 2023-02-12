#!/bin/bash


for f in *.cc *.h ; do

	echo "processing $f ..."
	cat $f | sed -e 's/JANA\/jerror/JANA\/Compatibility\/jerror/' | sed -e 's/\<DAQ\//rawdataparser\//g' | sed -e 's/JEventLoop\.h/JEvent\.h/g' | sed -e 's/JANA\/JStreamLog/JANA\/Compatibility\/JStreamLog/g' | sed -e 's/DANA\///g'> tmp.txt
	mv tmp.txt $f 

done

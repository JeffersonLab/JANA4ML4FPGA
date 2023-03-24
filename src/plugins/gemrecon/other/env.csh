#source /gluex/etc/hdonline.cshrc


#---------  GCC-4.9 -----------------

setenv MY_GXX_HOME /apps/gcc/4.9.2
setenv MY_PYT_HOME /apps/python/python-2.7.1

setenv PATH "${MY_GXX_HOME}/bin:${PATH}"

if ( $?LD_LIBRARY_PATH ) then
    echo "-- Attention: append  LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
    setenv LD_LIBRARY_PATH "${MY_GXX_HOME}/lib:${MY_GXX_HOME}/lib64:${LD_LIBRARY_PATH}"
else 
    setenv LD_LIBRARY_PATH "${MY_GXX_HOME}/lib:${MY_GXX_HOME}/lib64"
endif

setenv LD_RUN_PATH "${MY_GXX_HOME}/lib:${MY_GXX_HOME}/lib64:${LD_LIBRARY_PATH}"

#--- EVIO library single thread !!!

setenv EVIO     /home/hdtrdops/SRS/GemViewForJLab/evio_3.09
setenv GEM_LIB  /home/hdtrdops/furletov/SRS/gemrecon

#setenv LD_LIBRARY_PATH  ${EVIO}/src/libsrc/.Linux-x86_64-dbg:${EVIO}/src/libsrc++/.Linux-x86_64-dbg:$LD_LIBRARY_PATH
setenv LD_LIBRARY_PATH  ${GEM_LIB}:${EVIO}/Linux-x86_64/lib:$LD_LIBRARY_PATH

#-------- ROOT 5  with  Qt  ----------------

setenv ROOTSYS            /home/hdtrdops/LIBS
setenv PATH                $ROOTSYS/bin:$PATH
setenv LD_LIBRARY_PATH     $ROOTSYS/lib:$LD_LIBRARY_PATH
echo "set ROOTSYS=$ROOTSYS"




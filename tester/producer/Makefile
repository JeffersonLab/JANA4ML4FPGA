#---------------------------------------------------------------

ARCH          = linux

CXX           =
ObjSuf        = o
SrcSuf        = cc
ExeSuf        = exe
DllSuf        = so
OutPutOpt     = -o 

EVENTLIB      = $(EVENTSO)

#ROOTCFLAGS   := $(shell root-config --cflags)
#ROOTLIBS     := $(shell root-config --libs)
#ROOTGLIBS    := $(shell root-config --glibs)

ifeq ($(ARCH),linux)
# Linux with gcc 2.7.2.x
#---------------------------------------
#PROFILING = -O3 -finline-functions -finline-functions-called-once -finline-limit=2000 \
#         -funsafe-loop-optimizations 
#         -floop-optimize2 -funsafe-loop-optimizations
PROFILING = -O3
#PROFILING = -O3  -msse4 -DINTEL_SSE4 
#PROFILING = -O3  -msse4 -DINTEL_SSE4 -std=c99
#PROFILING = -O0 -g -Wno-write-strings
#PROFILING = -O0 -g
#PROFILING = -O0 -g -pg
#
#gprof    event-builder-ff.exe gmon.out > analysis3.txt
#gprof -b event-builder-ff.exe gmon.out > analysis3a.txt
#
#---------------------------------------
FFLAGS   = -Wall -c  -fno-automatic 
#---------------------------------------
CC      = g++
CXX     = g++
CFLAGS   = -Wall -fPIC -c    $(PROFILING)
CXXFLAGS = -Wall -fPIC  $(PROFILING)
LD            = g++
LDFLAGS       =   -Wall -fPIC  $(PROFILING)
CPP    =$(CXX) -E 
#---------------------------------------
SOFLAGS       = -shared
endif


CXXFLAGS     += $(ROOTCFLAGS)
LIBS          = $(ROOTLIBS)  -lThread  $(SYSLIBS) -lgfortran
GLIBS         = $(ROOTGLIBS) -lThread  $(SYSLIBS) -lgfortran
#------------------------------------------------------------

#PROGRAMS      = Monitor hist-srv  evb shmdep shmevb cdaq  tluc tlus
PROGRAMS      =  dep  ffe 
#------------------------------------------------------------
all:            $(PROGRAMS)

#............................................................
dep:
	rm .depend
	touch .depend
	$(CPP) -M -DCF__LINUX  *.cc *.cpp *.c > .depend
#	$(CPP) -M -DCF__LINUX -I $(ROOTCFLAGS) *.cc *.cpp *.c > .depend
include .depend
#...............................................................


TCPEVT	=	TCPclient
TCPEVTO	= 	TCPclient.$(ObjSuf)

$(MON):	$(FRO) $(CLO) $(DFO) $(MFRAO) $(MONO)  $(TCPEVTO) jf.o
	$(LD) $(LDFLAGS) $^  $(GLIBS) $(OutPutOpt)$@.exe  -Wl,-Map,$@.map
	@echo "$@ done"


#------------------------------------------------------------------------------

tcplib.o: tcplib.c tcplib.h
TCPclient.o:  TCPclient.cpp TCPclient.h
fifo.o: fifo.cc fifo.h 
event-builder.o: event-builder.cc event-builder.hh shmem_gluex.h
rc_client.o: rc_client.cc  shmem_gluex.h event-builder.hh tcplib.h 
tcp_thread.o:  tcp_thread.cc tcp_thread.h tcplib.h
hbook.o:  hbook.cc hbook.h shmem_gluex.h

#------------------------------------------------------------------------------
#           event server stuff
#------------------------------------------------------------------------------

.f.o:
	gfortran $(FFLAGS)   $<
.c.o:
	gcc $(CFLAGS)   $<
#.cc.o:
#	$(CXX) $(CFLAGS)   $<

#-------------------------------------------------------------------------------------------
#                 new 
#-------------------------------------------------------------------------------------------

FF  =  tcplib.o TCPclient.o  fifo.o event-builder.o  rc_client.o tcp_thread.o hbook.o

dc:	$(FF) shmem_gluex.h
	g++  $(LDFLAGS)  $(FF) -lpthread -lm   -o event-builder-dc.exe
	sync

eb:	$(FF) shmem_gluex.h
	g++  $(LDFLAGS)  $(FF) -lpthread -lm   -o event-builder-eb.exe
	sync

er:	$(FF) shmem_gluex.h
	g++  $(LDFLAGS)  $(FF) -lpthread -lm   -o event-builder-er.exe
	sync

evb:	$(FF) shmem_gluex.h
	g++  $(LDFLAGS)  $(FF) -lpthread -lm   -o event-builder-evb.exe
	sync
#-------------------------------------------------------------------------------------------

CLNT  = TCPclient.o TCPclient_rol2.o 
clnt:	$(CLNT)
	g++   $(LDFLAGS)  $(CLNT) -lm -o TCPclient.exe
	sync

#------------------------------------------------------------------------------
.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)
.PHONY:    Aclock Hello Tetris
#-------------------------------------------------------------
clean:
	@rm -f $(OBJS) core  *.o *.exe

distclean:	clean
	@rm -f $(PROGRAMS) $(EVENTSO) $(EVENTLIB) *Dict.* *.def *.exp \
	*.root *.ps *.so .def so_locations

.SUFFIXES: .$(SrcSuf)

###


.$(SrcSuf).$(ObjSuf):
	@echo "------------------>>>   different  compiler flags  !!!"
	$(CXX) $(CXXFLAGS) -c $<

test:
	@echo OBJS=$(OBJS)

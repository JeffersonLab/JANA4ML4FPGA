################################################################################ definition of the compiler options
#	-I location of directory containing include files
#	-L location of directory containing libraries
#       -lname include the library from -L location called libname.a
#	   -lg2c is the library containing info on converting fortran to C
#          -lf   is the library containing the intrinsic for HPUX only.
#	-shared make a shared library as output
#	-fPIC produce position independent code
#        necessary on some platforms (including HPUX) for -shared
# 	-fpic ^^ same(?)
#	-O optimizes
#	-g produces output for the debugger
#       -pg produces output for gprof profiler
#       note: if you want to see all warnings and ensure ansi standard 
#             compatibility, use:
#             -pipe -ansi -pedantic -fnonnull-objects \
#             -W -Wall -Wwrite-strings -Wpointer-arith -Wnested-externs \
#             -Woverloaded-virtual -Wbad-function-cast -fnonnull-objects
#       The proper order for cernlib libraries is:
#       -lpawlib -lgraflib -lgrafX11 -lmathlib -lkernlib -lpacklib -ljetset74
#
# makefile syntax:
#        for target thedir/target.suf from source anotherdir/source.suf2
#        ${*D}  = thedir
#        ${*F}  = target
#        $*     = thedir/target
#        $@     = thedir/target.suf
#        $<     = anotherdir/source.suf2
#  
#
################################################################################ Compiler options

EVIO_DIR = /home/hdtrdops/SRS/GemViewForJLab/evio_3.09
EVIO_INC = $(EVIO_DIR)/common/include/
EVIO_LIB = $(EVIO_DIR)/Linux-x86_64/lib/

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLIBS     := $(shell root-config --libs)
ROOTGLIBS    := $(shell root-config --glibs)

INCLUDES    = -I.   -I$(EVIO_INC)

CXX     =  g++
F77	=  gfortran

CXXFLAGS     =    -std=c++11 -fPIC  $(INCLUDES) $(ROOTCFLAGS)

LINK_FLAGS   =   $(DFLG) -fno-second-underscore

LINK_LIBS    =  $(ROOTLIBS)    -L$(EVIO_LIB) -levio -levioxx  -lexpat  

ifeq "$(UNAME)" "Linux"
ifeq "$(F77)" "gfortran"
    LINK_LIBS     += -lgfortran
else
    LINK_LIBS     += -lg2c 
endif
endif


HEADERS   = \
 GEMCluster.h\
 GemConfiguration.h\
 GemException.h\
 GemFEC.h\
 GemHistContainer.h\
 GEMHit.h\
 GemInputHandler.h\
 GemMapping.h\
 GEMOnlineHitDecoder.h\
 GEMPedestal.h\
 GEMRawDecoder.h\
 GEMRawPedestal.h\
 GemView.h\
 hardcode.h

PROGRAMS  = DAQCrateAnalyzer2

OBJGEM =  GEMRawDecoder.o GemFEC.o GemHistContainer.o GemView.o GemMapping.o GemConfiguration.o GemInputHandler.o GemException.o GEMOnlineHitDecoder.o GEMPedestal.o GEMHit.o GEMCluster.o GEMRawPedestal.o


###############################################################################
#
.SUFFIXES:      .o .cc .cxx .f .exe
#all:	lib $(PROGRAMS)
all:	lib 

#............................................................
dep:
	rm .depend
	touch .depend
	$(CXX)  $(CXXFLAGS)  -MM  *.cc  > .depend
include .depend
#...............................................................


DAQCrateAnalyzer2: DAQCrateAnalyzer2.o $(OBJGEM) $(HEADERS)
	@echo "Building $@ ..."
	$(CXX) $(LINK_FLAGS) DAQCrateAnalyzer2.o $(OBJGEM) $(LINK_LIBS) -o $@

###############################################################################
lib:       $(OBJGEM)
	ar rvs libgem.a $(OBJGEM)
	g++ -o libgem.so $(OBJGEM)  -shared

###############################################################################
# instructions for building a .o file from a .cxx file
#
.cc.o:         $(HEADERS) $<
	@echo "Compiling $< with $(CXX) ..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

###############################################################################
# instructions for building a .o file from a .cxx file
#
.c.o:         $(HEADERS) $<
	@echo "Compiling $< with $(CXX) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

###############################################################################
# instructions for building a .o file from a .f file
#
.f.o:           $<
	@echo "Compiling $< with $(F77) ..."
	@$(F77) $(FLAGS) -c $< -o $@

###############################################################################
# gmake clean       removes all garbage from HepMC directories.
#
clean:
	rm -f *.o

###############################################################################
# gmake distclean       removes all compiled libraries, executables, +garbage
#                       to prepare the package for distribution
distclean: 
	$(MAKE) clean --no-print-directory
	rm -f *.exe
	rm -f *.dat

test:
	@echo "CXXFLAGS = $(CXXFLAGS)"
	@echo "INCLUDES = $(INCLUDES)"	
	@echo "EVIO_DIR = $(EVIO_DIR)"
	@echo "EVIO_INC = $(EVIO_INC)"
	@echo "EVIO_LIB = $(EVIO_LIB)"
	@echo "HEADERS = $(HEADERS)"

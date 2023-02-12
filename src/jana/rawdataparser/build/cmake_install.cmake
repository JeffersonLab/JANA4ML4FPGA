# Install script for directory: /home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/hdtrdops/GemTrd_2023/JANA2/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/plugins/rawdataparser.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/plugins/rawdataparser.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/plugins/rawdataparser.so"
         RPATH "/u/apps/root/6.26.10/root-6.26.10-gcc9.3.0/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/plugins" TYPE SHARED_LIBRARY FILES "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/build/rawdataparser.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/plugins/rawdataparser.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/plugins/rawdataparser.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/plugins/rawdataparser.so"
         OLD_RPATH "/u/apps/root/6.26.10/root-6.26.10-gcc9.3.0/lib:"
         NEW_RPATH "/u/apps/root/6.26.10/root-6.26.10-gcc9.3.0/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/plugins/rawdataparser.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/rawdataparser" TYPE FILE FILES
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DBORptrs.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DBeamCurrent.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DBeamCurrent_factory.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DCODAControlEvent.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DCODAEventInfo.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DCODAROCInfo.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DDAQAddress.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DDAQConfig.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DEPICSvalue.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DEVIOWorkerThread.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DGEMSRSWindowRawData.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DL1Info.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DModuleType.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DParsedEvent.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DStatusBits.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DTSGBORConfig.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/DTSscalers.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250AsyncPedestal.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250BORConfig.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250Config.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250PulseData.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250PulseIntegral.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250PulsePedestal.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250PulseRawData.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250PulseTime.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250Scaler.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250StreamingRawData.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250TriggerTime.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250WindowRawData.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/Df250WindowSum.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/HDET.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/HDEVIO.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/JEventSourceGenerator_EVIOpp.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/JEventSource_EVIOpp.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/JFactoryGenerator_DAQ.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/LinkAssociations.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/async_filebuf.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/bor_roc.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/daq_param_type.h"
    "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/swap_bank.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/hdtrdops/GemTrd_2023/JANA4ML4FPGA/src/jana/rawdataparser/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")

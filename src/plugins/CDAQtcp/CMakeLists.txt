
cmake_minimum_required(VERSION 3.9)
project(janaTCP_project)

if(NOT "${CMAKE_CXX_STANDARD}")
  set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)   # Enable -fPIC for all targets

# Expose custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Set install directory to $JANA_HOME
set(CMAKE_INSTALL_PREFIX $ENV{JANA_HOME} CACHE PATH "magic incantation" FORCE)

# Find dependencies
find_package(JANA REQUIRED)


find_package(ROOT REQUIRED)

# According to the internet, CMake authors discourage the use
# of GLOB for identifying source files. IMHO, this is due to
# the flawed use of cache files in CMake itself. Here, GLOB
# is used as the default. What this means is you can add source
# files and re-run cmake (after clearing the cache file) and
# they will be found without needing to modify this file.
# You also have the option of switching the following to "false"
# and managing the source file list manually the way they recommend.
if(true)
  # Automatically determine source file list.
  file(GLOB mysourcefiles *.cpp *.cc *.c  *.hpp *.hh *.h)
  set( janaTCP_PLUGIN_SOURCES ${mysourcefiles} )    
else()
  # Manually manage source file list
  set (janaTCP_PLUGIN_SOURCES
        janaTCP.cc
  )
endif()

add_library(janaTCP_plugin SHARED ${janaTCP_PLUGIN_SOURCES})

target_include_directories(janaTCP_plugin PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(janaTCP_plugin ${JANA_LIB} ${ROOT_LIBRARIES})
set_target_properties(janaTCP_plugin PROPERTIES PREFIX "" OUTPUT_NAME "janaTCP" SUFFIX ".so")

install(TARGETS janaTCP_plugin DESTINATION plugins)

file(GLOB my_headers "*.h*")
install(FILES ${my_headers} DESTINATION include/janaTCP)

# For root dictionaries
file(GLOB my_pcms "${CMAKE_CURRENT_BINARY_DIR}/*.pcm")
install(FILES ${my_pcms} DESTINATION plugins)


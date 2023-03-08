cmake_minimum_required(VERSION 3.16)

project(JANA4ML4FPGA VERSION 0.0.0)

# Make C++17 a default
if(NOT "${CMAKE_CXX_STANDARD}")
    set(CMAKE_CXX_STANDARD 17)
endif()
message(STATUS "CMAKE_CXX_STANDARD = ${CMAKE_CXX_STANDARD}")

# Enable -fPIC for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find packages
find_package(JANA REQUIRED)
find_package(ROOT REQUIRED)

# Install to the top directory by default
if( ${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT} )
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install CACHE PATH "Install in top directory by default" FORCE)
endif()

set(PLUGIN_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/plugins")
set(INCLUDE_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/include")
set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/lib")
set(PROGRAM_OUTPUT_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")


message(STATUS "${CMAKE_PROJECT_NAME}: CMAKE_INSTALL_PREFIX    : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "${CMAKE_PROJECT_NAME}: PLUGIN_OUTPUT_DIRECTORY : ${PLUGIN_OUTPUT_DIRECTORY}")
message(STATUS "${CMAKE_PROJECT_NAME}: INCLUDE_OUTPUT_DIRECTORY: ${INCLUDE_OUTPUT_DIRECTORY}")
message(STATUS "${CMAKE_PROJECT_NAME}: LIBRARY_OUTPUT_DIRECTORY: ${LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "${CMAKE_PROJECT_NAME}: PROGRAM_OUTPUT_DIRECTORY: ${PROGRAM_OUTPUT_DIRECTORY}")


# JANA2
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 cmake   : ${JANA_DIR}")
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 includes: ${JANA_INCLUDE_DIR}")
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 library : ${JANA_LIBRARY}")


# CERN ROOT
message(STATUS "${CMAKE_PROJECT_NAME}: ROOT version  : ${ROOT_VERSION}")
message(STATUS "${CMAKE_PROJECT_NAME}: ROOT libraries: ${ROOT_LIBRARIES}")


# Add CMake additional functionality:
include(cmake/print_functions.cmake)                        # Helpers to print fancy headers, file names, etc
include(cmake/print_subdirectory_tree.cmake)                # Prints processed subdirectories
include(cmake/jana_plugin.cmake)                            # Add common settings for plugins
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)   # Find Find<Modules>.cmake


# Add src as a global include directory
include_directories(${CMAKE_SOURCE_DIR}/src)


# Add library directories to rpath so users don't need to use LD_LIBRARY_PATH.
# (see https://dev.my-gate.net/2021/08/04/understanding-rpath-with-cmake )
# TODO: detector MacOS and set MACOSX_RPATH to TRUE
set( CMAKE_SKIP_BUILD_RPATH FALSE )
set( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE )
set( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" )
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )


# Libraries are a separated objects.
# We add this include directory so that in code we could use
# #include <tcp_daq/tcp_event.h> instead of <libraries/tcp_daq/tcp_event.h>
# For peer plugins the later is OK, but for libraries <tcp_daq/tcp_event.h> is better
include_directories(${CMAKE_SOURCE_DIR}/src/libraries)

# ---------------------------------------------------------------------------
print_grand_header("    B U I L D   J A N A 4 M L 4 F P G A   P A R T S    ")
# ---------------------------------------------------------------------------


add_subdirectory( src/libraries )
add_subdirectory( src/services )
add_subdirectory(src/plugins)
add_subdirectory(src/executables)



# Print what we had built
print_header("CMake processed subdirectories:")
print_subdirectory_tree()
message(STATUS "\n-------------------------------")


# Install all cmake helpers
# include(CMakePackageConfigHelpers)
# configure_package_config_file(cmake/EICreconConfig.cmake.in cmake/EICreconConfig.cmake INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/EICrecon)
# install(FILES ${CMAKE_BINARY_DIR}/cmake/EICreconConfig.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/EICrecon)  # why is this needed?
# file(GLOB EICRECON_CMAKE_FILES cmake/*.cmake)
# install(FILES ${EICRECON_CMAKE_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/EICrecon)


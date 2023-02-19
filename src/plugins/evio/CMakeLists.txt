
cmake_minimum_required(VERSION 3.9)
project(EVIOSource)


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
  file(GLOB SRC_FILES *.cc *.h)
else()
  # Manually manage source file list
  set(SRC_FILES
          JEventSourceEVIOSource.cc
          HDEVIO.cc
          async_filebuf.cc
          ../../Extensions/spdlog/SpdlogMixin.h)
endif()

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${JANA_LIB} ${ROOT_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" OUTPUT_NAME ${PROJECT_NAME} SUFFIX ".so")

install(TARGETS ${PROJECT_NAME} DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})

file(GLOB my_headers "*.h*")
install(FILES ${my_headers} DESTINATION ${INCLUDE_OUTPUT_DIRECTORY}/${PROJECT_NAME})



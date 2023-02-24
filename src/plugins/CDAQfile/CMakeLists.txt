
# TODO: updae to match example_evio_analysis

cmake_minimum_required(VERSION 3.9)
project(CDAQfile)

# Manually manage source file list
set(SRC_FILES
        CDAQfile.cc
        CDAQEVIOFileSource.h
        ../../libraries/evio/HDEVIO.h
        ../../libraries/evio/HDEVIO.cc
        ../../libraries/evio/DModuleType.h
        ../../extensions/spdlog/SpdlogMixin.h)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${JANA_LIB} ${ROOT_LIBRARIES} evio)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" OUTPUT_NAME ${PROJECT_NAME} SUFFIX ".so")

install(TARGETS ${PROJECT_NAME} DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})



project(lib_evio)

print_header(">>>>   L I B R A R Y :   evio    <<<<")       # Fancy printing

# Find dependencies
find_package(JANA REQUIRED)

file(GLOB lib_evio_SRC
        *.h
        *.cc
        )

set(LIBRARY_NAME evio)

add_library(${LIBRARY_NAME} SHARED ${lib_evio_SRC})
target_include_directories(${LIBRARY_NAME}  PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR})

install(TARGETS ${LIBRARY_NAME}  DESTINATION ${LIBRARY_OUTPUT_DIRECTORY})

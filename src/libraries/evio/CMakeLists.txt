project(lib_evio)

print_header(">>>>   L I B R A R Y :   evio    <<<<")       # Fancy printing

# Find dependencies
find_package(JANA REQUIRED)

file(GLOB lib_evio_SRC
        *.h
        *.cc
        )

set(LIBRARY_NAME evio)

add_library(${LIBRARY_NAME} STATIC ${lib_evio_SRC})
set_target_properties(${LIBRARY_NAME}  PROPERTIES PREFIX "lib" OUTPUT_NAME "EVIO")
target_include_directories(${LIBRARY_NAME}  PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR})
target_link_libraries(${LIBRARY_NAME}  ${JANA_LIB})

install(TARGETS ${LIBRARY_NAME}  DESTINATION ${LIBRARY_OUTPUT_DIRECTORY})

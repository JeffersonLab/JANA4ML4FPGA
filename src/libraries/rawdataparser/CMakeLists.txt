project(lib_rawdataparser)

print_header(">>>>   L I B R A R Y :   rawdataparser    <<<<")       # Fancy printing

# Find dependencies
find_package(JANA REQUIRED)
find_package(ROOT REQUIRED)

# file(GLOB lib_rawdataparser_SRC
#         *.h
#         *.cc
#         )

set(lib_rawdataparser_SRC
        EVIOBlockedEventParser.cc
        EVIOBlockedEventParser.h
        swap_bank.cc
        )

set(LIBRARY_NAME rawdataparser)

add_library(${LIBRARY_NAME} STATIC ${lib_rawdataparser_SRC})
set_target_properties(${LIBRARY_NAME}  PROPERTIES PREFIX "lib" OUTPUT_NAME "rawdataparser")
target_include_directories(${LIBRARY_NAME}  PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(${LIBRARY_NAME}  ${JANA_LIB})

install(TARGETS ${LIBRARY_NAME}  DESTINATION ${LIBRARY_OUTPUT_DIRECTORY})


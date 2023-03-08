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

add_library(${LIBRARY_NAME} SHARED ${lib_rawdataparser_SRC})
target_include_directories(${LIBRARY_NAME}  PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})

install(TARGETS ${LIBRARY_NAME}  DESTINATION ${LIBRARY_OUTPUT_DIRECTORY})


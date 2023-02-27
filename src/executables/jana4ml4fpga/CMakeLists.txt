
project(jana4ml4fpga)

print_header(">>>>   E X E C U T A B L E S : ${PROJECT_NAME}   <<<<")

file(GLOB block_tester_SRC
        *.h
        *.cc
        )

add_executable(${PROJECT_NAME} ${block_tester_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${JANA_LIB} evio rawdataparser Threads::Threads dl)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}  DESTINATION ${PROGRAM_OUTPUT_DIRECTORY})

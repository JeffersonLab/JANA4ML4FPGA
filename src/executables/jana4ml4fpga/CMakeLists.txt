
project(jana4ml4fpga)

print_header(">>>>   E X E C U T A B L E S : ${PROJECT_NAME}   <<<<")

file(GLOB block_tester_SRC
        *.h
        *.cc
        )

message("{JANA_LIB} ${JANA_LIB}")
message("{{JANA_INCLUDE_DIR}} ${JANA_INCLUDE_DIR}")

add_executable(${PROJECT_NAME} ${block_tester_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} evio rawdataparser ${JANA_LIB}  Threads::Threads dl ${ROOT_LIBRARIES})
target_link_options(${PROJECT_NAME} PRIVATE -rdynamic)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}  DESTINATION ${PROGRAM_OUTPUT_DIRECTORY})

remove_definitions()


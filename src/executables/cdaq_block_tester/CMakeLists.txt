
project(block_tester)

print_header(">>>>   E X E C U T A B L E S : block_tester   <<<<")

file(GLOB block_tester_SRC
        *.h
        *.cc
        )

add_executable(${PROJECT_NAME} block_tester_SRC)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${JANA_LIB} evio)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" OUTPUT_NAME ${PROJECT_NAME} SUFFIX ".so")

install(TARGETS ${PROJECT_NAME}  DESTINATION ${PROGRAM_OUTPUT_DIRECTORYq})

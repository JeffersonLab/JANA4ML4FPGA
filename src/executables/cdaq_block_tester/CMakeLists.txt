
project(block_tester)
set(CMAKE_CXX_STANDARD 17)

print_header(">>>>   E X E C U T A B L E S : ${PROJECT_NAME}   <<<<")


set(block_tester_SRC
        main.cc
        EVIOBlockedEvent.h
        EVIOBlockedEventSource.h
        EVIOBlockProcessor.h
        EVIOBlockProcessor.cc
        )

add_executable(${PROJECT_NAME} ${block_tester_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} ${JANA_LIB} evio pthread)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}  DESTINATION ${PROGRAM_OUTPUT_DIRECTORYq})

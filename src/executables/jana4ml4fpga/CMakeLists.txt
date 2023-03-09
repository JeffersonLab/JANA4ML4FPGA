print_header(">>>>   E X E C U T A B L E S : ${PROJECT_NAME}   <<<<")

project(jana4ml4fpga)

file(GLOB block_tester_SRC
        *.h
        *.cc
        )

add_executable(${PROJECT_NAME} ${block_tester_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${JANA_LIB} evio rawdataparser Threads::Threads dl ${ROOT_LIBRARIES})

# get spdlog and fmt
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${fmt_DIR}/../../../include)
target_link_libraries(${PROJECT_NAME} spdlog::spdlog fmt::fmt)

# install target
target_link_libraries(${PROJECT_NAME} evio rawdataparser JANA::jana2_static_lib  Threads::Threads dl ${ROOT_LIBRARIES})
target_link_options(${PROJECT_NAME} PRIVATE -rdynamic)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME}  DESTINATION ${PROGRAM_OUTPUT_DIRECTORY})

remove_definitions()


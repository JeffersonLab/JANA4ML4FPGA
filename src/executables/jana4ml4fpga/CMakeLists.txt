project(jana4ml4fpga)

print_header(">>>>   E X E C U T A B L E S : ${PROJECT_NAME}   <<<<")

file(GLOB jana4ml4fpga_SRC
        *.h
        *.cc
        )

add_executable(${PROJECT_NAME} ${jana4ml4fpga_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${JANA_INCLUDE_DIR}
        ${fmt_DIR}/../../../include
        )

target_link_libraries(${PROJECT_NAME}
        JANA::jana2_static_lib
        spdlog::spdlog fmt::fmt
        evio rawdataparser Threads::Threads dl ${ROOT_LIBRARIES}
        )

target_link_options(${PROJECT_NAME} PRIVATE -rdynamic)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}  DESTINATION ${PROGRAM_OUTPUT_DIRECTORY})

remove_definitions()

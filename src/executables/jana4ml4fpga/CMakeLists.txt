
project(jana4ml4fpga)

print_header(">>>>   E X E C U T A B L E S : ${PROJECT_NAME}   <<<<")

file(GLOB jana4ml4fpga_SRC
        *.h
        *.cc
        )

# get spdlog and fmt
#find_package(spdlog REQUIRED)
#find_package(fmt REQUIRED)
#target_include_directories(${PROJECT_NAME} PUBLIC ${fmt_DIR}/../../../include)
#target_link_libraries(${PROJECT_NAME} spdlog::spdlog fmt::fmt)

add_executable(${PROJECT_NAME} ${jana4ml4fpga_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${JANA_INCLUDE_DIR}
        ${fmt_DIR}/../../../include
        )

target_link_libraries(${PROJECT_NAME}
        ${JANA_LIB}
        spdlog::spdlog fmt::fmt
        evio rawdataparser Threads::Threads dl ${ROOT_LIBRARIES}
        )


target_link_options(${PROJECT_NAME} PRIVATE -rdynamic)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME}  DESTINATION ${PROGRAM_OUTPUT_DIRECTORY})


# Add CERN ROOT
if(NOT ROOT_FOUND)
    find_package(ROOT REQUIRED)
endif()
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${ROOT_INCLUDE_DIRS} )
target_link_libraries(${PROJECT_NAME} ${ROOT_LIBRARIES})

#remove_definitions()

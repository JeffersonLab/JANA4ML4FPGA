# Automatically set plugin name the same as the directory name
# Don't forget string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in its name
get_filename_component(DIRECOTRY_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
    print_header(">>>>   U N I T   T E S T S :   ${DIRECOTRY_NAME}    <<<<")       # Fancy printing


# These tests can use the Catch2-provided main
add_executable(${DIRECOTRY_NAME}
        algorithms_test.cc
        catch_amalgamated.cpp
        catch_amalgamated.hpp
        rolling_average_tests.cpp
        average_tests.cpp
        standard_deviation_test.cpp
        )

# These tests need their own main
#add_executable(custom-main-tests test.cpp test-main.cpp)
#target_link_libraries(custom-main-tests PRIVATE Catch2::Catch2)

# Install executable
install(TARGETS ${DIRECOTRY_NAME} DESTINATION bin)

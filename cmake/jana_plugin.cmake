macro(target_add_common_dependencies _name)
    # Include JANA by default
    if(NOT JANA_FOUND)
        find_package(JANA REQUIRED)
    endif()

    # include fmt by default
    if(NOT fmt_FOUND)
        find_package(fmt REQUIRED)
    endif()

    # include logging by default
    if(NOT spdlog_FOUND)
        find_package(spdlog REQUIRED)
    endif()

    # include ROOT by default
    if(NOT ROOT_FOUND)
        find_package(ROOT REQUIRED)
    endif()

    target_include_directories(${_name} PUBLIC ${CMAKE_SOURCE_DIR}/src)
    target_include_directories(${_name} SYSTEM PUBLIC ${JANA_INCLUDE_DIR} )
    target_include_directories(${_name} SYSTEM PUBLIC ${ROOT_INCLUDE_DIRS} )
    target_include_directories(${_name} PUBLIC ${fmt_DIR}/../../../include)
    target_link_libraries(${_name} ${JANA_LIB} spdlog::spdlog fmt::fmt)

endmacro()

# Common macro to add plugins
macro(plugin_add _name)
    project(${_name}_project)

    # Define plugin
    add_library(${_name}_plugin SHARED ${PLUGIN_SOURCES})

    # Convention: plugins don't have 'lib' prefix
    set_target_properties(${_name}_plugin PROPERTIES PREFIX "" OUTPUT_NAME "${_name}" SUFFIX ".so")

    # Where to install plugins
    install(TARGETS ${_name}_plugin DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})

    # add common dependencies
    target_add_common_dependencies(${_name}_plugin)
endmacro()


# target_link_libraries for both a plugin and a library
macro(plugin_link_libraries _name)
    target_link_libraries(${_name}_plugin ${ARGN})
endmacro()


# target_include_directories for both a plugin and a library
macro(plugin_include_directories _name)
    target_include_directories(${_name}_plugin  ${ARGN})
endmacro()


# runs target_sources both for library and a plugin
macro(plugin_sources _name)
    # This is needed as this is a macro (see cmake macro documentation)
    set(SOURCES ${ARGN})

    # Add sources to plugin
    target_sources(${_name}_plugin PRIVATE ${SOURCES})
endmacro()

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
macro(plugin_glob_all _name)

    # But... GLOB here makes this file just hot pluggable
    file(GLOB LIB_SRC_FILES *.cc *.cpp *.c)
    file(GLOB PLUGIN_SRC_FILES *.cc *.cpp *.c)
    file(GLOB HEADER_FILES *.h *.hh *.hpp)

    # We need plugin relative path for correct headers installation
    string(REPLACE ${CMAKE_SOURCE_DIR}/src "" PLUGIN_RELATIVE_PATH ${PROJECT_SOURCE_DIR})

    # Add sources to plugin
    target_sources(${_name}_plugin PRIVATE ${PLUGIN_SRC_FILES} ${HEADER_FILES})

    #Add correct headers installation
    # Install headers for plugin
    install(FILES ${HEADER_FILES} DESTINATION include/${PLUGIN_RELATIVE_PATH})

    # >oO Debug output if needed
    if(${EICRECON_VERBOSE_CMAKE})
        message(STATUS "plugin_glob_all:${_name}: PLUGIN_CC_FILE   ${PLUGIN_CC_FILE}")
        message(STATUS "plugin_glob_all:${_name}: LIB_SRC_FILES    ${LIB_SRC_FILES}")
        message(STATUS "plugin_glob_all:${_name}: PLUGIN_SRC_FILES ${PLUGIN_SRC_FILES}")
        message(STATUS "plugin_glob_all:${_name}: HEADER_FILES     ${HEADER_FILES}")
        message(STATUS "plugin_glob_all:${_name}: PLUGIN_RLTV_PATH ${PLUGIN_RELATIVE_PATH}")
    endif()

    # To somehow control GLOB lets at least PRINT files we are going to compile:
    message(STATUS "Source files:")
    print_file_names("  " ${PLUGIN_SRC_FILES})    # Prints source files
    message(STATUS "Plugin-main file is: ${PLUGIN_CC_FILE}")
    message(STATUS "Header files:")
    print_file_names("  " ${HEADER_FILES})  # Prints header files

endmacro()


# Adds dd4hep for a plugin
macro(plugin_add_dd4hep _name)

    if(NOT DD4hep_FOUND)
        find_package(DD4hep REQUIRED)
    endif()

    plugin_include_directories(${_name} SYSTEM PUBLIC ${DD4hep_INCLUDE_DIRS})
    plugin_link_libraries(${_name} DD4hep::DDCore DD4hep::DDRec)

endmacro()


# Adds ACTS tracking package for a plugin
macro(plugin_add_acts _name)

    if(NOT Acts_FOUND)
        find_package(Acts REQUIRED COMPONENTS Core PluginIdentification PluginTGeo PluginJson PluginDD4hep)
        set(Acts_VERSION_MIN "19.0.0")
        set(Acts_VERSION "${Acts_VERSION_MAJOR}.${Acts_VERSION_MINOR}.${Acts_VERSION_PATCH}")
        if(${Acts_VERSION} VERSION_LESS ${Acts_VERSION_MIN}
                AND NOT "${Acts_VERSION}" STREQUAL "9.9.9")
            message(FATAL_ERROR "Acts version ${Acts_VERSION_MIN} or higher required, but ${Acts_VERSION} found")
        endif()

        set(Acts_INCLUDE_DIRS ${Acts_DIR}/../../../include ${ActsDD4hep_DIR}/../../../include )
    endif()

    # Add include directories (works same as target_include_directories)
    plugin_include_directories(${_name}_plugin SYSTEM PUBLIC ${Acts_INCLUDE_DIRS})

    # Add libraries (works same as target_include_directories)
    plugin_link_libraries(${_name}_plugin ActsCore ActsPluginIdentification ActsPluginTGeo ActsPluginJson ActsPluginDD4hep)
endmacro()


# Adds IRT PID reconstruction package for a plugin
macro(plugin_add_irt _name)
    if(NOT IRT_FOUND)
        find_package(IRT REQUIRED)
        set(IRT_INCLUDE_DIR ${IRT_DIR}/../../include)
    endif()
    plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${IRT_INCLUDE_DIR})
    plugin_link_libraries(${PLUGIN_NAME} IRT)
endmacro()

# Adds podio, edm4hep, edm4eic for a plugin
macro(plugin_add_event_model _name)

    if(NOT podio_FOUND)
        find_package(podio REQUIRED)
    endif()

    if(NOT EDM4HEP_FOUND)
        find_package(EDM4HEP REQUIRED)
    endif()

    if(NOT EDM4EIC_FOUND)
        find_package(EDM4EIC REQUIRED)
        set(EDM4EIC_INCLUDE_DIR ${EDM4EIC_DIR}/../../include)
    endif()

    # Add include directories
    # (same as target_include_directories but for both plugin and library)
    plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${podio_INCLUDE_DIR} ${EDM4EIC_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR})

    # Add libraries
    # (same as target_include_directories but for both plugin and library)
    plugin_link_libraries(${PLUGIN_NAME}
            EDM4EIC::edm4eic
            EDM4HEP::edm4hep
            )
endmacro()


# Adds cern ROOT for a plugin
macro(plugin_add_cern_root _name)

    if(NOT ROOT_FOUND)
        #find_package(ROOT REQUIRED COMPONENTS Core Tree Hist RIO EG)
        find_package(ROOT REQUIRED)
    endif()

    # Add include directories
    plugin_include_directories(${_name} SYSTEM PUBLIC ${ROOT_INCLUDE_DIRS} )

    # Add libraries
    #plugin_link_libraries(${PLUGIN_NAME} ${ROOT_LIBRARIES} EDM4EIC::edm4eic algorithms_digi_library algorithms_tracking_library ROOT::EG)
    plugin_link_libraries(${_name} ${ROOT_LIBRARIES} ROOT::EG)
endmacro()

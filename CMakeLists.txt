# Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
# Author: Renat R. Dusaev <crank@qcrypt.org>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#
# Common CMake
#############

cmake_minimum_required( VERSION 2.6 )
project( StromaV )

set( StromaV_VERSION_MAJOR 0  )
set( StromaV_VERSION_MINOR 1  )
set( StromaV_VERSION_PATCH dev )
set( StromaV_VERSION ${StromaV_VERSION_MAJOR}.${StromaV_VERSION_MINOR}.${StromaV_VERSION_PATCH} )

list(APPEND CMAKE_MODULE_PATH "${ROOT_DIR}/etc/root/cmake"
                              "${ROOT_DIR}/etc/cmake" )

#
# CMake customization
####################

# Goo library contains general infrastructure layer constituing application
# logic: logging, config files, exception handling, etc. We need to find Goo
# right here because of a subset of its useful CMake-modules.
find_package( Goo REQUIRED )
# This will define a dependant-options check function upon build:
include(${Goo_CMAKE_MODULES_DIR}/OptionDepend.cmake)
# This will check for out-of-source build:
include(${Goo_CMAKE_MODULES_DIR}/OutOfSource.cmake)
AssureOutOfSourceBuilds()
# This will provide push_option() macro
include(${Goo_CMAKE_MODULES_DIR}/OptionList.cmake)

# Options
########

# Below are build options that can be enabled or disabled by
# -D<option_name>=<ON|OFF> option given to CMake.
# The \option tag is used by a special script, so do not remove these
# comments.
#\option:
push_option( GEANT4_MC_MODEL
        "Enable Geant4 integration."
        OFF )
#\option:
push_option( GEANT4_DYNAMIC_PHYSICS
        "Enable run-time physlist composition for Geant4."
        OFF )
#\option:
push_option( G4_MDL_VIS
        "Enable Geant4 visualization manager."
        OFF )
#\option:
push_option( G4_MDL_GUI
        "Enable Geant4 GUI."
        OFF )
#\option:
push_option( RPC_PROTOCOLS
        "Eneables RPC serialization protocols."
        OFF )
#\option:
push_option( ANALYSIS_ROUTINES
        "Enable analysis routines."
        OFF )
#\option
push_option( ALIGNMENT_ROUTINES
        "Enable alignment routines."
        OFF )
#\option
push_option( PYTHON_BINDINGS
        "Enable python bindings (requires SWIG)."
        OFF )
#\option
push_option( DSuL
        "Enable detector selection micro-language (requires YACC/LEXX)"
        ON )
push_option( DOC_HTML
    "Whether to produce HTML documentation with doxygen."
        OFF )

# check for options deps in form (dependent, [dependencies ...])
# The \opt-dep: tag is used by buildtest.sh script.
#\opt-dep:
option_depend( G4_MDL_GUI               GEANT4_MC_MODEL )
#\opt-dep:
option_depend( G4_MDL_VIS               GEANT4_MC_MODEL )
#\opt-dep:
option_depend( GEANT4_DYNAMIC_PHYSICS   GEANT4_MC_MODEL )
#\opt-dep
option_depend( ALIGNMENT_ROUTINES       RPC_PROTOCOLS )


#
# Third-party libraries and packages
###################################

include( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sV_find_pkgs.cmake )

if( NOT PKG_CONFIG_FOUND OR NOT yaml-cpp_FOUND )
    message( FATAL_ERROR "
    Unable to locate required package yaml-cpp.
    Please submit the path to yaml-cpp-config.cmake with
    -Dyaml-cpp_DIR=..." )
else()
    message( STATUS "yaml-cpp found with pkg-config: ${yaml-cpp_INCLUDE_DIRS}" )
endif()

if( RPC_PROTOCOLS )
    protobuf_generate_cpp(GPROTO_MSGS_SRCS GPROTO_MSGS_HDRS
                          event.proto)
endif( RPC_PROTOCOLS )

# Doxygen is a nice and robust software for automated creation of
# documentation.
if( DOC_HTML )
    find_package( Doxygen REQUIRED )
endif( DOC_HTML )

# Detector selector micro-language may be generated if BISON/FLEX are
# available
if( DSuL )
    BISON_TARGET(DSuL_Parser assets/grammar/dsul.y
        ${CMAKE_CURRENT_BINARY_DIR}/dsul_parser_generated.c
        #VERBOSE ${CMAKE_CURRENT_BINARY_DIR}/dsul.dbg
        COMPILE_FLAGS "--graph=${CMAKE_CURRENT_BINARY_DIR}/dsul.dot" )
    FLEX_TARGET(DSuL_Lexer   assets/grammar/dsul.l
        ${CMAKE_CURRENT_BINARY_DIR}/dsul_lexer_generated.c)
    ADD_FLEX_BISON_DEPENDENCY(DSuL_Lexer DSuL_Parser)
    #message( "XXX: ${BISON_DSuL_Parser_OUTPUTS} ${FLEX_DSuL_Lexer_OUTPUTS}" )
endif( DSuL )

if( CTEMPLATE_FOUND )
    set( TEMPLATED_LOGGING TRUE )
else( CTEMPLATE_FOUND )
    message( WARNING "Ctemplate library wasn't found. That is not fatal, but \
    some template options within logging facility will not be available \
    yielding much less informative output." )
    set( TEMPLATED_LOGGING FALSE )
endif( CTEMPLATE_FOUND )

#
# SET-UP BUILD-TIME PARAMETERS
#############################

site_name( BUILDER_HOSTNAME )

# Obtain versioning parameters
EXECUTE_PROCESS(COMMAND date +"%x %x"
                OUTPUT_VARIABLE BUILD_TIMESTAMP
                OUTPUT_STRIP_TRAILING_WHITESPACE )
set( StromaV_LIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/inc/" )

# Configure compiler -- base flags
#set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -D_GNU_SOURCE -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -DHAVE_INLINE -std=c99" )
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -D_GNU_SOURCE -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -std=gnu++11 -Wno-c99-extensions" )
#if( CMAKE_COMPILER_IS_GNUCXX )
#    set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -rdynamic" )
#    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic" )
#endif( CMAKE_COMPILER_IS_GNUCXX )

set( CMAKE_C_FLAGS_DEBUG            "${CMAKE_C_FLAGS_DEBUG}     -Wfatal-errors -fno-omit-frame-pointer" ) # -Wfatal-errors -fprofile-use? -fprofile-correction
set( CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}   -Wfatal-errors -fno-omit-frame-pointer" )
set( CMAKE_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE}" )
set( CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}" )
set( CMAKE_C_FLAGS_MINSIZEREL       "${CMAKE_C_FLAGS_MINSIZEREL}" )
set( CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}" )
set( CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO} -Wfatal-errors" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -Wfatal-errors" )

# Attain configuration depending on current build dir.
include(${Goo_CMAKE_MODULES_DIR}/BuildConf.cmake)
set( StromaV_LIB StromaV${StromaV_BUILD_POSTFIX} )

#
# Include directories
####################

#
# Geant4 pushes include directories with -I flag; we need, however,
# them to be included with -isystem to avoid annoying warnings. This
# tweak will move all the include dir previously pushed by third-party
# modules from usual INCLUDE_DIRECTORIES list to
# INTERFACE_SYSTEM_INCLUDE_DIRECTORIES.
get_property(_supp_inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES "")
# This lines is used for dev needs. Print out all the include dirs:
#get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#foreach(dir ${dirs})
#    message(STATUS "I-dir='${dir}'")
#endforeach()
#get_property(sdirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES)
#foreach(dir ${sdirs})
#    message(STATUS "S-dir='${dir}'")
#endforeach()

if( PYTHON_BINDINGS AND RPC_PROTOCOLS )
    find_package( PythonLibs REQUIRED )
endif( PYTHON_BINDINGS AND RPC_PROTOCOLS )

#
# Link directories
#################

#link_directories( ${extGDML_LIBRARY_DIR} )
#if( genfit_LIBRARY_DIR )
#    link_directories( ${genfit_LIBRARY_DIR} )
#endif( genfit_LIBRARY_DIR )
#link_directories( ${yaml-cpp_LIBRARY_DIRS} )

#
# Configure sources
##################

# Physics list aux config
if( GEANT4_MC_MODEL )
    if( NOT DEFINED Geant4_PHLIST_INCLUDE_DIR )
        set( Geant4_PHLIST_INCLUDE_DIR ${Geant4_INCLUDE_DIR} )
    endif(NOT DEFINED Geant4_PHLIST_INCLUDE_DIR )

    include( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/g4_dynamic_physics.cmake )

    # Phys. lists:
    collect_Geant4_definitions(
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_phLists.sh
        "${StromaV_LIB_INCLUDE_DIR}/g4extras/auto.in/PhysList.ihpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/inc/g4extras/auto.out/PhysList.ihpp" "" )
    # Particles:
    collect_Geant4_definitions(
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_g4Particles.sh
        "${StromaV_LIB_INCLUDE_DIR}/g4extras/auto.in/Particles.ihpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/inc/g4extras/auto.out/Particles.ihpp" G4 )
    # Physics modules:
    collect_Geant4_definitions(
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_physics.sh
        "${StromaV_LIB_INCLUDE_DIR}/g4extras/auto.in/Physics.ihpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/inc/g4extras/auto.out/Physics.ihpp" "" )

    # Remove this dirty things outta my compiler options!
    string(REPLACE "-pedantic"              "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
    string(REGEX REPLACE "-Wno-[^\ ]*"      "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
endif( GEANT4_MC_MODEL )

# TARGETS
########

#
# If installation prefix is default, set it to /usr/local
if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "default install path" FORCE )
    message( STATUS "NOTE: Install prefix is set to \""
        ${CMAKE_INSTALL_PREFIX} "\" as it was set to default. "
        "The -rpath won't be specified as it appears to be a "
        "system path."
        "To override this specification, use -DCMAKE_INSTALL_PREFIX=<path> "
        "option." )
    set( StromaV_CONFIG_INSTALL_PATH /etc/StromaV )
else( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    message( STATUS "NOTE: Install prefix is manually set to \""
        ${CMAKE_INSTALL_PREFIX} "\"." )
    # ...
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/StromaV")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    # the RPATH to be used when installing, but only if it's not a system directory
    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
    set( StromaV_CONFIG_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/etc/StromaV )
endif( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )

set( DEFAULT_STROMAV_CFG_PATH ${StromaV_CONFIG_INSTALL_PATH}
    CACHE STRING "Default config file for StromaV library.")
set( StromaV_ASSETS_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/share/StromaV )
set( StromaV_MODULES_INSTALL_PATH
    ${CMAKE_INSTALL_PREFIX}/lib/StromaV/modules )
set( StromaV_ROOT_PLUGINS_INSTALL_PATH
    ${CMAKE_INSTALL_PREFIX}/lib/StromaV/modules/root )

#
# Add custom uninstall target (CMake by default does not provide it)
configure_file(
    "${Goo_CMAKE_MODULES_DIR}/CMakeUninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target( uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake") 

# generic StromaV lib configuration
configure_file (
    "${StromaV_LIB_INCLUDE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/inc/sV_config.h"
)

#
# Library target
file(GLOB_RECURSE StromaV_LIB_SOURCES src/*.c*)

if( DSuL )
    list( APPEND StromaV_LIB_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/dsul_parser_generated.c
        ${CMAKE_CURRENT_BINARY_DIR}/dsul_lexer_generated.c )
endif( DSuL )

add_library( ${StromaV_LIB} SHARED
        ${StromaV_LIB_SOURCES}
        $<$<BOOL:${RPC_PROTOCOLS}>:${GPROTO_MSGS_SRCS}>
        #$<$<BOOL:${ANALYSIS_ROUTINES}>:TrackingGroupDict.cxx>
        $<$<BOOL:${DSuL}>:${BISON_DSuL_Parser_OUTPUTS}>
        $<$<BOOL:${DSuL}>:${FLEX_DSuL_Lexer_OUTPUTS}>
    )
# Why does CLANG ignores this sometimes?
target_include_directories( ${StromaV_LIB}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/inc>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/rootPlugins>
            $<INSTALL_INTERFACE:include/StromaV> )
target_include_directories( ${StromaV_LIB} SYSTEM
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/inc>
        PUBLIC
            #${extGDML_INCLUDE_DIRS}/..
            ${GSL_INCLUDE_DIRS}
            ${Boost_INCLUDE_DIRS}
            ${ROOT_INCLUDE_DIRS}
            ${yaml-cpp_INCLUDE_DIRS}
            ${OPENSSL_INCLUDE_DIR}
            $<$<BOOL:${CTEMPLATE_FOUND}>:${CTEMPLATE_INCLUDE_DIR}>
            $<$<BOOL:${ZLIB_FOUND}>:${ZLIB_INCLUDE_DIRS}>
            $<$<BOOL:${RPC_PROTOCOLS}>:${PROTOBUF_INCLUDE_DIRS}>
            $<$<BOOL:${GEANT4_MC_MODEL}>:${Geant4_INCLUDE_DIR}>
            ${_supp_inc_dirs} )

if( genfit_FOUND )
    target_include_directories( ${StromaV_LIB} PUBLIC ${genfit_INCLUDE_DIRS} )
endif( genfit_FOUND )

if( PYTHON_BINDINGS AND RPC_PROTOCOLS )
    target_include_directories( ${StromaV_LIB} SYSTEM PUBLIC ${PYTHON_INCLUDE_DIRS} )
    target_link_libraries( ${StromaV_LIB} PUBLIC ${PYTHON_LIBRARIES} )
endif( PYTHON_BINDINGS AND RPC_PROTOCOLS )

#add_dependencies( ${StromaV_LIB} ${PY_MSG_MODULES} )  # XXX
set_target_properties( ${StromaV_LIB} PROPERTIES VERSION ${StromaV_VERSION}
                       SOVERSION ${StromaV_VERSION_MAJOR} )
target_compile_features( ${StromaV_LIB} PUBLIC
            c_variadic_macros
            cxx_constexpr
            cxx_final
            cxx_delegating_constructors
            cxx_variadic_macros
            cxx_variadic_templates
            cxx_template_template_parameters )

install( TARGETS ${StromaV_LIB} EXPORT StromaV LIBRARY DESTINATION lib/StromaV )
install( DIRECTORY inc/ DESTINATION include/StromaV FILES_MATCHING PATTERN
    "*.h" PATTERN "*.hpp" PATTERN "*.tcc" PATTERN "*.itcc" )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/inc/sV_config.h
         DESTINATION include/StromaV )
if( RPC_PROTOCOLS )
    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/event.pb.h
             DESTINATION include/StromaV )
endif( RPC_PROTOCOLS )

target_link_libraries( ${StromaV_LIB}
    PUBLIC ${Goo_LIBRARIES}
    PUBLIC ${Boost_LIBRARIES}
    PUBLIC ${ROOT_LIBRARIES}
    PUBLIC ${extGDML_LIBRARIES}
    PUBLIC ${yaml-cpp_LIBRARIES}
    PRIVATE ${OPENSSL_LIBRARIES} )
if( CTEMPLATE_FOUND )
    target_link_libraries( ${StromaV_LIB} PUBLIC ${CTEMPLATE_LIBRARIES})
endif( CTEMPLATE_FOUND )

if( GEANT4_MC_MODEL )
    target_link_libraries( ${StromaV_LIB} PUBLIC ${Geant4_LIBRARIES})
    if( G4_MDL_GUI )
        target_link_libraries( ${StromaV_LIB} PRIVATE ${QT_LIBRARIES})
    endif( G4_MDL_GUI )
endif( GEANT4_MC_MODEL )
if( ANALYSIS_ROUTINES )
    target_link_libraries( ${StromaV_LIB} PRIVATE ${GSL_LIBRARIES} )
endif( ANALYSIS_ROUTINES )
if( RPC_PROTOCOLS )
    set_source_files_properties(${GPROTO_MSGS_SRCS} PROPERTIES COMPILE_FLAGS -w )
    set_source_files_properties(${GPROTO_MSGS_SRCS} PROPERTIES GENERATED TRUE )
    target_link_libraries( ${StromaV_LIB}
            PUBLIC  ${PROTOBUF_LIBRARIES}
                    ${Protobuf_LIBRARIES}
                    ${Protobuf_LITE_LIBRARIES}  )
    if( RFIO_AKA_SHIFT_LIB )
        message( STATUS "Found RFIO library: "  )
        target_link_libraries( ${StromaV_LIB} PUBLIC ${RFIO_AKA_SHIFT_LIB} )
    endif(RFIO_AKA_SHIFT_LIB )
    if( DATE_LIBMONITOR )
        message( STATUS "Found DATE/libmonitor library: " ${DATE_LIBMONITOR} )
        target_link_libraries( ${StromaV_LIB} PUBLIC ${DATE_LIBMONITOR} )
    endif( DATE_LIBMONITOR )
endif( RPC_PROTOCOLS )

if( genfit_FOUND )
    target_link_libraries( ${StromaV_LIB} PUBLIC ${genfit_LIBRARIES} )
endif( genfit_FOUND )

if( DOC_HTML )
    configure_file (
        ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    )
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif( DOC_HTML )

#
# Py-bindings
if( PYTHON_BINDINGS AND RPC_PROTOCOLS )
    add_subdirectory( py )
    # Can fix possible issue with target visibility:
    set_source_files_properties( ${PY_MSG_MODULES} PROPERTIES GENERATED TRUE )
endif( PYTHON_BINDINGS AND RPC_PROTOCOLS )

#
# ROOT bindings
if( ANALYSIS_ROUTINES )
    ## TODO: figure out which target does the ROOT_GENERATE_DICTIONARY add
    ## to change it to the target_include_directories()
    #get_target_property( _Goo_INCS ${Goo_LIBRARIES} INTERFACE_INCLUDE_DIRECTORIES )
    #include_directories( TrackingGroupDict.cxx
    #                     "inc"
    #                     ${_Goo_INCS}
    #                     ${CMAKE_CURRENT_BINARY_DIR}/inc )
    #ROOT_GENERATE_DICTIONARY( TrackingGroupDict
    #    ${PROJECT_SOURCE_DIR}/inc/alignment/TrackingGroup*
    #    LINKDEF alignment/TrackingGroupLinkDef.h )
    include( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ROOTAuxDicts.cmake )
    sV_ROOT_dictionaries( TrackingGroupDict
        HEADERS inc/alignment/TrackingGroup*
        LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/inc/alignment/TrackingGroupLinkDef.h
        TARGET_LIB ${StromaV_LIB} )
endif( ANALYSIS_ROUTINES )

# Build ROOT plugins
#\project-subdir:
add_subdirectory( rootPlugins )
# Prepare rootrc appending some environment variables:
configure_file (
        ${PROJECT_SOURCE_DIR}/StromaV-plugins.rootrc.in
        ${CMAKE_CURRENT_BINARY_DIR}/StromaV-plugins.rootrc
    )

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

# in order to obtain build_* options, the options_list() has to be invoked
# AFTER the add_subdirectory() which performs configuration of appropriate
# utils.
#\project-subdir:
add_subdirectory( apps )

options_list()  #< collects the ${PKG_NAME}_OPTIONS_STR variable

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/StromaVConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/StromaVConfig.cmake"
    @ONLY
)

configure_file (
    conf/00_sV.yml.in
    ${CMAKE_CURRENT_BINARY_DIR}/00_sV.yml
    @ONLY
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/StromaVConfigVersion.cmake
    VERSION ${StromaV_VERSION}
  COMPATIBILITY AnyNewerVersion
)
export( EXPORT StromaV
    FILE ${CMAKE_CURRENT_BINARY_DIR}/StromaVTargets.cmake
)
install( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/StromaVConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/StromaVTargets.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/StromaVConfigVersion.cmake
    cmake/FindCtemplate.cmake
    cmake/sV_find_pkgs.cmake
    cmake/ROOTAuxDicts.cmake
    DESTINATION lib/cmake/StromaV )

install( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/00_sV.yml
         DESTINATION ${StromaV_CONFIG_INSTALL_PATH} )
install( DIRECTORY conf/ DESTINATION ${StromaV_CONFIG_INSTALL_PATH} FILES_MATCHING PATTERN
    "*.yml" PATTERN "*.yaml" PATTERN "*.conf" )

install( FILES
         "${CMAKE_CURRENT_SOURCE_DIR}/event.proto"
         DESTINATION share/StromaV )

install( FILES
         ${CMAKE_CURRENT_BINARY_DIR}/StromaV-plugins.rootrc
         DESTINATION ${StromaV_ROOT_PLUGINS_INSTALL_PATH} )

install(
    CODE "file( GLOB _GeneratedROOTDicts \"${CMAKE_CURRENT_BINARY_DIR}/*.pcm\" )"
    CODE "file( INSTALL \${_GeneratedROOTDicts} DESTINATION \"${StromaV_ROOT_PLUGINS_INSTALL_PATH}\" )"
)

install( EXPORT StromaV DESTINATION lib/cmake/StromaV
    FILE StromaVTargets.cmake )


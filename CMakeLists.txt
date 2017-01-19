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

list( APPEND CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "/etc/root/cmake" )  # <- This one is a new ROOT convention.

#
# CMake customization
####################

# Goo library contains general infrastructure layer constituing application
# logic: logging, config files, exception handling, etc. We need to find Goo
# right here because of a subset of its useful CMake-modules.
find_package( Goo REQUIRED )
find_package( extGDML REQUIRED )
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
push_option( DOC_HTML
    "Whether to produce HTML documentation with doxygen."
        OFF )

set( DEFAULT_STROMAV_CFG_PATH "../../StromaV/defaults.cfg"
    CACHE STRING "Default config file for StromaV library.")

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
# THIRD-PARTY LIBRARIES AND PACKAGES
###################################

# ROOT framework is a major data analysis toolkit used in many applications
# inside of StromaV
find_package( ROOT
              COMPONENTS Eve Gui Ged Geom RGL EG REQUIRED )

# Boost library is common for most modern Linux distros. Implements a lot
# of extended functionality: containers, threading, I/O, etc.
find_package( Boost ${Boost_FORCE_VERSION}
              COMPONENTS program_options
                         system
                         thread
                         unit_test_framework
                         iostreams
              REQUIRED )

# StromaV uses Google Protocol Buffers as a base layer for event structure
# representation. This dependency is optional and needed only if event
# processing pipeline is enabled.
if( RPC_PROTOCOLS )
    find_package( Protobuf REQUIRED )
    #set(PROTOBUF_GENERATE_CPP_APPEND_PATH FALSE)
    protobuf_generate_cpp(GPROTO_MSGS_SRCS GPROTO_MSGS_HDRS
                          event.proto)
endif( RPC_PROTOCOLS )

# GEANT4 is a framework for Monte-Carlo simulations in high-energy and nuclear
# physics. This dependency is optional and have to be enabled only if you're
# planning to perform simulations.
if( GEANT4_MC_MODEL )
    find_package(Geant4)
    if( G4_MDL_GUI )
        find_package(Geant4 REQUIRED ui_all vis_all)
    else( G4_MDL_GUI )
        find_package(Geant4 REQUIRED)
    endif( G4_MDL_GUI )
    include(${Geant4_USE_FILE})
endif( GEANT4_MC_MODEL )

# Qt is a GUI widgets framework available in most of modern distributives. It
# is somehow bound to Geant4 on its GUI layer, so one have to link against it
# if planning to integrate Geant4's GUI.
if( GEANT4_MC_MODEL )
    if( G4_MDL_GUI )
        set( DESIRED_QT_VERSION "4" )
        find_package(Qt)
    endif( G4_MDL_GUI )
endif( GEANT4_MC_MODEL )

# Genfit (optional) is a track reconstruction software utilizing Kalman
# filter. It is not a common package and should be built in a custom way.
# We have a fork for it here: https://github.com/GenFit/GenFit providing
# the CMake configuration module.
if( ANALYSIS_ROUTINES )
    find_package( GenFit REQUIRED )
endif( ANALYSIS_ROUTINES )

# GSL (GNU Scientific Library) is a common package available on most modern
# Linux distributives. Provides a wide set of fast, robust and well-tested
# scientific numerical routines for interpolation, approximation, linear
# algebra and so on. It is only required if StromaV's analysis routines is
# needed.
if( ANALYSIS_ROUTINES )
    find_package(GSL REQUIRED)
endif( ANALYSIS_ROUTINES )

# Doxygen is a nice and robust software for automated creation of
# documentation.
if( DOC_HTML )
    find_package( Doxygen REQUIRED )
endif( DOC_HTML )


#
# SET-UP BUILD-TIME PARAMETERS
#############################

site_name( BUILDER_HOSTNAME )

# Insist to make CMake do not cache these variables:
mark_as_advanced( BUILDER_HOSTNAME DAQDECODING_INCLUDE_DIR DAQDECODING_LIB
    DESIRED_QT_VERSION GSL_CONFIG_EXECUTABLE
    Geant4_DIR QT_QMAKE_EXECUTABLE
    ROOT_CINT_EXECUTABLE ROOT_CONFIG_EXECUTABLE
    Goo_INCLUDE_DIRS Goo_LIBRARIES )

# Obtain versioning parameters
EXECUTE_PROCESS(COMMAND date +"%x %x"
                OUTPUT_VARIABLE BUILD_TIMESTAMP
                OUTPUT_STRIP_TRAILING_WHITESPACE )
set( StromaV_LIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/inc/" )

# Configure compiler -- base flags
set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -D_GNU_SOURCE -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -DHAVE_INLINE -std=c99" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -D_GNU_SOURCE -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -std=gnu++11 -Wno-c99-extensions" )

if( CMAKE_COMPILER_IS_GNUCXX )
    set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -rdynamic" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic" )
endif( CMAKE_COMPILER_IS_GNUCXX )

set( CMAKE_C_FLAGS_DEBUG            "${CMAKE_C_FLAGS_DEBUG}     -fno-omit-frame-pointer" ) # -Wfatal-errors -fprofile-use? -fprofile-correction
set( CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}   -fno-omit-frame-pointer" )
set( CMAKE_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE}" )
set( CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}" )
set( CMAKE_C_FLAGS_MINSIZEREL       "${CMAKE_C_FLAGS_MINSIZEREL}" )
set( CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}" )
set( CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO}" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )

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
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES "")
include_directories( SYSTEM ${dirs} )
# This lines is used for dev needs. Print out all the include dirs:
#get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#foreach(dir ${dirs})
#    message(STATUS "I-dir='${dir}'")
#endforeach()
#get_property(sdirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES)
#foreach(dir ${sdirs})
#    message(STATUS "S-dir='${dir}'")
#endforeach()

# Why does CLANG ignores this sometimes?
include_directories( "inc"
                     "rootPlugins"
                     SYSTEM ${Goo_INCLUDE_DIRS}
                     ${extGDML_INCLUDE_DIRS}/..
                     ${GSL_INCLUDE_DIRS}
                     ${Boost_INCLUDE_DIRS}
                     ${ROOT_INCLUDE_DIRS}
                     $<$<BOOL:${RPC_PROTOCOLS}>:${PROTOBUF_INCLUDE_DIRS}>
                     $<$<BOOL:${RPC_PROTOCOLS}>:${CMAKE_CURRENT_BINARY_DIR}>
                     $<$<BOOL:${GEANT4_MC_MODEL}>:${Geant4_INCLUDE_DIR}> )
if( genfit_INCLUDE_DIRS )
    include_directories( ${genfit_INCLUDE_DIRS} )
endif( genfit_INCLUDE_DIRS )


#
# Link directories
#################

link_directories( ${Goo_LIBRARY_DIR} )

if( genfit_LIBRARY_DIR )
    link_directories( ${genfit_LIBRARY_DIR} )
endif( genfit_LIBRARY_DIR )

#
# Configure sources
##################

# generic StromaV lib configuration
configure_file (
    "${StromaV_LIB_INCLUDE_DIR}/config.h.in"
    "${StromaV_LIB_INCLUDE_DIR}/config.h"
)

# Physics list aux config
if( GEANT4_MC_MODEL )
    if( NOT DEFINED Geant4_PHLIST_INCLUDE_DIR )
        set( Geant4_PHLIST_INCLUDE_DIR ${Geant4_INCLUDE_DIR} )
    endif(NOT DEFINED Geant4_PHLIST_INCLUDE_DIR )
    execute_process(COMMAND
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_phLists.sh
        ${Geant4_PHLIST_INCLUDE_DIR}
        OUTPUT_VARIABLE G4PHYSLISTS )
    # Gather physics list:
    if( G4PHYSLISTS )
        string(REPLACE "\n" "," G4PHYSLISTS ${G4PHYSLISTS})
        message( STATUS "Found pre-defined PhysLists: \n" ${G4PHYSLISTS} )
        string(REPLACE ","  ";" G4PHYSLISTS ${G4PHYSLISTS})
        foreach( phLst ${G4PHYSLISTS} )
            set( G4_PHLIST_INCLUDES_STR
                "${G4_PHLIST_INCLUDES_STR}\n# include <${phLst}.hh>" )
            set( G4_PHLIST_DEFINES_STR
                "${G4_PHLIST_DEFINES_STR}    m(${phLst}) \\\n" )
            # TODO: conditionally determine if we really need to re-generate
            # it as now each `$ make' invokation causes rebuild.
            configure_file (
                "${StromaV_LIB_INCLUDE_DIR}/g4extras/auto.in/PhysList.ihpp.in"
                "${StromaV_LIB_INCLUDE_DIR}/g4extras/auto.out/PhysList.ihpp"
            )
        endforeach( phLst )
    else( G4PHYSLISTS )
        message( SEND_ERROR "No physics list found in \
        ${Geant4_INCLUDE_DIR}. May indicate problems with \
        your Geant4 installation." )
    endif( G4PHYSLISTS )
    if( GEANT4_DYNAMIC_PHYSICS )
        # Gather particles:
        execute_process(COMMAND
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_g4Particles.sh
            ${Geant4_INCLUDE_DIR}
            OUTPUT_VARIABLE G4PARTICLES )
        if( G4PARTICLES )
            string(REPLACE "\n" "," G4PARTICLES ${G4PARTICLES})
            message( STATUS "Found pre-defined Particles: \n" ${G4PARTICLES} )
            string(REPLACE ","  ";" G4PARTICLES ${G4PARTICLES})
            foreach( particle ${G4PARTICLES} )
                set( G4_PARTICLES_INCLUDES_STR
                    "${G4_PARTICLES_INCLUDES_STR}\n# include <G4${particle}.hh>" )
                set( G4_PARTICLES_DEFINES_STR
                    "${G4_PARTICLES_DEFINES_STR}    m(${particle}) \\\n" )
                # TODO: conditionally determine if we really need to re-generate
                # it as now each `$ make' invokation causes rebuild.
                configure_file (
                    "${StromaV_LIB_INCLUDE_DIR}/g4extras/auto.in/Particles.ihpp.in"
                    "${StromaV_LIB_INCLUDE_DIR}/g4extras/auto.out/Particles.ihpp"
                )
            endforeach( particle )
        else( G4PARTICLES )
            message( SEND_ERROR "No particle definitions found in \
            ${Geant4_INCLUDE_DIR}. May indicate problems with \
            your Geant4 installation." )
        endif( G4PARTICLES )
        # Gather physics modules:
        execute_process(COMMAND
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_physics.sh
            ${Geant4_INCLUDE_DIR}
            OUTPUT_VARIABLE G4PHYSICS )
        if( G4PHYSICS )
            string(REPLACE "\n" "," G4PHYSICS ${G4PHYSICS})
            message( STATUS "Found pre-defined Physics: \n" ${G4PHYSICS} )
            string(REPLACE ","  ";" G4PHYSICS ${G4PHYSICS})
            foreach( physics ${G4PHYSICS} )
                set( G4_PHYSICS_INCLUDES_STR
                    "${G4_PHYSICS_INCLUDES_STR}\n# include <${physics}.hh>" )
                set( G4_PHYSICS_DEFINES_STR
                    "${G4_PHYSICS_DEFINES_STR}    m(${physics}) \\\n" )
                # TODO: conditionally determine if we really need to re-generate
                # it as now each `$ make' invokation causes rebuild.
                configure_file (
                    "${StromaV_LIB_INCLUDE_DIR}/g4extras/auto.in/Physics.ihpp.in"
                    "${StromaV_LIB_INCLUDE_DIR}/g4extras/auto.out/Physics.ihpp"
                )
            endforeach( physics )
        else( G4PHYSICS )
            message( SEND_ERROR "No physics definitions found in \
            ${Geant4_INCLUDE_DIR}. May indicate problems with \
            your Geant4 installation." )
        endif( G4PHYSICS )
    endif( GEANT4_DYNAMIC_PHYSICS )
endif( GEANT4_MC_MODEL )

# Remove this dirty things outta my compiler options!
string(REPLACE "-pedantic"              "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
string(REPLACE "-Wno-long-long"         "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
string(REPLACE "-Wno-non-virtual-dtor"  "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
string(REPLACE "-Wno-c99-extensions"    "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )

# Add ROOT dictionaries
if( ANALYSIS_ROUTINES )
    ROOT_GENERATE_DICTIONARY( TrackingGroupDict
        ${PROJECT_SOURCE_DIR}/inc/alignment/TrackingGroup*
        LINKDEF alignment/TrackingGroupLinkDef.h )
endif( ANALYSIS_ROUTINES )

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
else( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    message( STATUS "NOTE: Install prefix is manually set to \""
        ${CMAKE_INSTALL_PREFIX} "\"." )
    # ...
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/StromaV")
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    # the RPATH to be used when installing, but only if it's not a system directory
    LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
endif( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )

#
# Add custom uninstall target (CMake by default does not provide it)
configure_file(
    "${Goo_CMAKE_MODULES_DIR}/CMakeUninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target( uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake") 

#
# Library target
file(GLOB_RECURSE StromaV_LIB_SOURCES src/*.c*)

add_library( ${StromaV_LIB} SHARED
        ${StromaV_LIB_SOURCES}
        $<$<BOOL:${RPC_PROTOCOLS}>:${GPROTO_MSGS_SRCS}>
        $<$<BOOL:${ANALYSIS_ROUTINES}>:TrackingGroupDict.cxx>
    )

#add_dependencies( ${StromaV_LIB} ${PY_MSG_MODULES} )  # XXX

set_target_properties( ${StromaV_LIB} PROPERTIES VERSION ${StromaV_VERSION}
                       SOVERSION ${StromaV_VERSION_MAJOR} )

install( TARGETS ${StromaV_LIB} LIBRARY DESTINATION lib/StromaV )
install( DIRECTORY inc/ DESTINATION include/StromaV FILES_MATCHING PATTERN
    "*.h" PATTERN "*.hpp" PATTERN "*.tcc" PATTERN "*.itcc" )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/event.pb.h
         DESTINATION include/StromaV )

target_link_libraries( ${StromaV_LIB} ${Goo_LIBRARIES}   )
target_link_libraries( ${StromaV_LIB} ${Boost_LIBRARIES} )
target_link_libraries( ${StromaV_LIB} ${ROOT_LIBRARIES}  )
if( GEANT4_MC_MODEL )
    target_link_libraries( ${StromaV_LIB} ${Geant4_LIBRARIES})
    if( G4_MDL_GUI )
        target_link_libraries( ${StromaV_LIB} ${QT_LIBRARIES})
    endif( G4_MDL_GUI )
endif( GEANT4_MC_MODEL )
if( ANALYSIS_ROUTINES )
    target_link_libraries( ${StromaV_LIB} ${GSL_LIBRARIES} )
endif( ANALYSIS_ROUTINES )
if( RPC_PROTOCOLS )
    set_source_files_properties(${GPROTO_MSGS_SRCS} PROPERTIES COMPILE_FLAGS -Wno-shadow)
    target_link_libraries( ${StromaV_LIB} ${PROTOBUF_LIBRARIES} )
    if( RFIO_AKA_SHIFT_LIB )
        message( STATUS "Found RFIO library: "  )
        target_link_libraries( ${StromaV_LIB} ${RFIO_AKA_SHIFT_LIB} )
    endif(RFIO_AKA_SHIFT_LIB )
    if( DATE_LIBMONITOR )
        message( STATUS "Found DATE/libmonitor library: " ${DATE_LIBMONITOR} )
        target_link_libraries( ${StromaV_LIB} ${DATE_LIBMONITOR} )
    endif( DATE_LIBMONITOR )
endif( RPC_PROTOCOLS )
if( genfit_LIBRARIES )
    target_link_libraries( ${StromaV_LIB} ${genfit_LIBRARIES} )
endif( genfit_LIBRARIES )

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
if( PYTHON_BINDINGS )
    add_subdirectory( py )
    # Can fix possible issue with target visibility:
    #set_source_files_properties( ${PY_MSG_MODULES} GENERATED TRUE )
endif( PYTHON_BINDINGS )

# Build ROOT plugins
#\project-subdir:
add_subdirectory( rootPlugins )
# Prepare rootrc appending some environment variables:
configure_file (
        "${PROJECT_SOURCE_DIR}/StromaV-plugins.rootrc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/StromaV-plugins.rootrc"
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

options_list()
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/StromaVConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/StromaVConfig.cmake"
    @ONLY
)

install( FILES
    "${CMAKE_CURRENT_BINARY_DIR}/StromaVConfig.cmake"
         DESTINATION lib/cmake/StromaV )

install( FILES
         "${CMAKE_CURRENT_SOURCE_DIR}/defaults.cfg"
         DESTINATION etc/StromaV )

install( FILES
         "${CMAKE_CURRENT_SOURCE_DIR}/event.proto"
         DESTINATION share/StromaV )


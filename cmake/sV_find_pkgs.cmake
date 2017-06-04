# Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
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
# Third-party libraries and packages required by StromaV

find_package( Goo REQUIRED )
find_package( ext.gdml REQUIRED )

# YAML-cpp provides config parsing facility dealing with flexible YAML fmt
find_package( yaml-cpp QUIET )
if( NOT yaml-cpp_FOUND )
    find_package( PkgConfig )
    if( PKG_CONFIG_FOUND )
        pkg_search_module( yaml-cpp REQUIRED yaml-cpp )
    endif( PKG_CONFIG_FOUND )
    if( NOT PKG_CONFIG_FOUND OR NOT yaml-cpp_FOUND )
        message( FATAL_ERROR "
        Unable to locate required package yaml-cpp.
        Please submit the path to yaml-cpp-config.cmake with
        -Dyaml-cpp_DIR=..." )
    endif()
endif()
# ROOT framework is a major data analysis toolkit used in many applications
# inside of StromaV
find_package( ROOT
              COMPONENTS Eve Gui Ged Geom RGL EG REQUIRED )

# Boost library is common for most modern Linux distros. Implements a lot
# of extended functionality: containers, threading, I/O, etc.
find_package( Boost ${Boost_FORCE_VERSION}
              COMPONENTS system
                         thread
                         unit_test_framework
                         iostreams
              REQUIRED )

# StromaV uses Google Protocol Buffers as a base layer for event structure
# representation. This dependency is optional.
if( RPC_PROTOCOLS OR StromaV_RPC_PROTOCOLS )
    find_package( Protobuf REQUIRED )
    #set(PROTOBUF_GENERATE_CPP_APPEND_PATH FALSE)
    # TODO: protobuf_generate_cpp(GPROTO_MSGS_SRCS GPROTO_MSGS_HDRS
    #                      event.proto)
endif( RPC_PROTOCOLS OR StromaV_RPC_PROTOCOLS )

# GEANT4 is a framework for Monte-Carlo simulations in high-energy and nuclear
# physics. This dependency is optional and have to be enabled only if you're
# planning to perform simulations.
if( GEANT4_MC_MODEL OR StromaV_GEANT4_MC_MODEL )
    find_package(Geant4)
    if( G4_MDL_GUI )
        find_package(Geant4 REQUIRED ui_all vis_all)
    else( G4_MDL_GUI )
        find_package(Geant4 REQUIRED)
    endif( G4_MDL_GUI )
    include(${Geant4_USE_FILE})
endif( GEANT4_MC_MODEL OR StromaV_GEANT4_MC_MODEL )

# Qt is a GUI widgets framework available in most of modern distributives. It
# is somehow bound to Geant4 on its GUI layer, so one have to link against it
# if planning to integrate Geant4's GUI.
if( GEANT4_MC_MODEL OR StromaV_GEANT4_MC_MODEL )
    if( G4_MDL_GUI )
        set( DESIRED_QT_VERSION "4" )
        find_package(Qt)
    endif( G4_MDL_GUI )
endif( GEANT4_MC_MODEL OR StromaV_GEANT4_MC_MODEL )

# Genfit (optional) is a track reconstruction software utilizing Kalman
# filter. It is not a common package and should be built in a custom way.
# We have a fork for it here: https://github.com/GenFit/GenFit providing
# the CMake configuration module.
if( ANALYSIS_ROUTINES OR StromaV_ANALYSIS_ROUTINES )
    find_package( GenFit REQUIRED )
endif( ANALYSIS_ROUTINES OR StromaV_ANALYSIS_ROUTINES )

# GSL (GNU Scientific Library) is a common package available on most modern
# Linux distributives. Provides a wide set of fast, robust and well-tested
# scientific numerical routines for interpolation, approximation, linear
# algebra and so on. It is only required if StromaV's analysis routines is
# needed.
if( ANALYSIS_ROUTINES OR StromaV_ANALYSIS_ROUTINES )
    find_package(GSL REQUIRED)
endif( ANALYSIS_ROUTINES OR StromaV_ANALYSIS_ROUTINES )

# Detector selector micro-language may be generated if BISON/FLEX are
# available
if( DSuL )
    find_package(BISON REQUIRED)
    find_package(FLEX REQUIRED)
    #BISON_TARGET(DSuL_Parser assets/grammar/dsul.y
    #    ${CMAKE_CURRENT_SOURCE_DIR}/src/dsul_parser_generated.c
    #    VERBOSE ${CMAKE_CURRENT_BINARY_DIR}/dsul.dbg
    #    COMPILE_FLAGS "--graph=${CMAKE_CURRENT_BINARY_DIR}/dsul.dot" )
    #FLEX_TARGET(DSuL_Lexer   assets/grammar/dsul.l
    #    ${CMAKE_CURRENT_SOURCE_DIR}/src/dsul_lexer_generated.c)
    #ADD_FLEX_BISON_DEPENDENCY(DSuL_Lexer DSuL_Parser)
    ##message( "XXX: ${BISON_DSuL_Parser_OUTPUTS} ${FLEX_DSuL_Lexer_OUTPUTS}" )
endif( DSuL )

# Compression formats
find_package( ZLIB )  # ZLIB_FOUND
find_package( BZip2 )  # BZIP2_FOUND
find_package( LibLZMA )  # LIBLZMA_FOUND
# ^^^ Note: the project is now called XZ utils, but library, headers, and all
# the CMake name suffixes are still keeping the name LZMA.

# OpenSSL is used for computing advanced hashes
find_package( OpenSSL REQUIRED )

# Tries to find Ctemplate library and headers:
get_filename_component(_CURRENT_LOCATION "${CMAKE_CURRENT_LIST_FILE}" PATH)
include( ${_CURRENT_LOCATION}/FindCtemplate.cmake )
#if( NOT CTEMPLATE_FOUND )
#    message( WARNING "Ctemplate library wasn't found. That is not fatal, but \
#    some template options within logging facility will not be available \
#    yielding much less informative output." )
#    set( TEMPLATED_LOGGING FALSE )
#else( NOT CTEMPLATE_FOUND )
#    set( TEMPLATED_LOGGING TRUE )
#endif( NOT CTEMPLATE_FOUND )


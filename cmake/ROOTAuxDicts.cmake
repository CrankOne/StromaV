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

# Extended version of native ROOT's ROOT_GENERATE_DICTIONARY() function
# performing automated resolution of include directories associated with target
# library.
#
# Function signature:
# sV_ROOT_dictionary(   <dictionary-name>
#                       HEADERS <header1>...
#                       LINKDEF <*LinkDef.h>
#                       [INCLUDE_DIRS <inc1>...]
#                       [OPTIONS <CLING-opts>...]
#                       [TARGET_LIB <lib>]
#                       [DO_KEEP_PATHS]
#                       [INTERPRETER_ONLY] )
# The TARGET_LIB argument shall refer to the shared library (preferably a
# previously-defined target) that will contain a corresponding class(es)
# implementation(s). If this library was defined as a target, the corresponding
# include directories will be appended to the includes list of CLING executable
# arguments.
#
# The .pcm file is fundamental for the correct functioning of the dictionary at
# runtime. It should be located in the directory where the shared library is
# installed in which the compiled dictionary resides.
#
# The corresponding *LinkDef.h header files will be located automatically and
# specified to rootcling invokation.
# 
# The DO_KEEP_PATHS will disable the -noIncludePaths option making the
# rootcling store include directories paths.
#
# The INTERPRETER_ONLY corresponds to -interpreteronly argument of rootcling.
function( sV_ROOT_dictionaries dictionary )
    # Function signature:
    set( options DO_KEEP_PATHS INTERPRETER_ONLY )
    set( oneValueArgs LINKDEF TARGET_LIB )
    set( multiValueArgs HEADERS OPTIONS INCLUDE_DIRS )
    # Parse arguments:
    cmake_parse_arguments( ROOT_dict__ARGS
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    if( NOT ROOT_FOUND )
        find_package( ROOT )
    endif( NOT ROOT_FOUND )
    if( NOT ROOT_FOUND )
        message( FATAL_ERROR "ROOT distribution not found. Unable to generate dictionary \"${dictionary}\"." )
    endif( NOT ROOT_FOUND )

    # Obtain the include directories list from linked library if it is a
    # CMake target:
    if( TARGET ${ROOT_dict__ARGS_TARGET_LIB} )
        get_target_property( _LIB_INCS ${ROOT_dict__ARGS_TARGET_LIB} INTERFACE_INCLUDE_DIRECTORIES )
        get_target_property( _LIB_SINCS ${ROOT_dict__ARGS_TARGET_LIB} INTERFACE_SYSTEM_INCLUDE_DIRECTORIES )
    else()
        if( ROOT_dict__ARGS_TARGET_LIB )
            message( STATUS "Library \"${ROOT_dict__ARGS_TARGET_LIB}\" provided to rootcling \
is not a CMake target, so no include directories will be obtained." )
        endif()
    endif()
    get_directory_property( _THIS_DIR_INCDIRS INCLUDE_DIRECTORIES )
    foreach( incdir IN LISTS _THIS_DIR_INCDIRS _LIB_INCS _LIB_SINCS ROOT_dict__ARGS_INCLUDE_DIRS )
        list( APPEND _INCLUDE_DIRS ${incdir} )
    endforeach()

    # Check, whether any include directory were provided:
    if( NOT _INCLUDE_DIRS )
        message( WARNING "No include directories will be provided to rootcling." )
    else()
        list( REMOVE_DUPLICATES _INCLUDE_DIRS )
    endif()

    foreach( d ${_INCLUDE_DIRS})
        set(includedirs_str ${includedirs_str} -I${d})
    endforeach()

    # Form the input headers list:
    foreach( hdr IN LISTS ${ROOT_dict__ARGS_HEADERS} )
        if(${hdr} MATCHES "[*?]") # Is this header a globbing expression?
            file( GLOB_RECURSE _hdrs ${hdr} FOLLOW_SYMLINKS )
            foreach(f ${_hdrs})
                if(NOT f MATCHES LinkDef) # skip LinkDefs from globbing result
                    set(headerfiles ${headerfiles} ${f})
                endif()
            endforeach()
        else()
            find_file( f ${hdr} HINTS ${_INCLUDE_DIRS} NO_DEFAULT_PATH )
            set( headerfiles ${headerfiles} ${f} )
            unset( f CACHE )
        endif()
    endforeach()

    # Check, if the provided *LinkDef.h file is reachable, and if not, try to
    # locate it within the existing include directories.
    if( EXISTS ${ROOT_dict__ARGS_LINKDEF} )
        set( _LINKDEF_FILE ${ROOT_dict__ARGS_LINKDEF} )
    else()
        unset( _LINKDEF_FILE CACHE )
        get_filename_component( _PRFX ${ROOT_dict__ARGS_LINKDEF} DIRECTORY )
        get_filename_component( _FNME ${ROOT_dict__ARGS_LINKDEF} NAME )
        message( STATUS "DBG1: ${_PRFX} :: ${_FNME} :: ${_INCLUDE_DIRS}" )
        find_file( _LINKDEF_FILE NAMES ${_FNME} ${ROOT_dict__ARGS_LINKDEF}
            PATHS ${_INCLUDE_DIRS}
            PATH_SUFFIXES ${_PRFX}
            NO_DEFAULT_PATH )
        message( "DBG2: ${_LINKDEF_FILE}" )
    endif()

    if( NOT _LINKDEF_FILE )
        message( FATAL_ERROR "Unable to find LinkDef file \
for dictionary \"${dictionary}\" specified as \"${ROOT_dict__ARGS_LINKDEF}\"." )
    else()
        message( STATUS "LinkDef file for dictionary \"${dictionary}\" found: ${_LINKDEF_FILE}" )
    endif()

    set( OPTS ${ROOT_dict__ARGS_OPTIONS} )
    if( NOT ${ROOT_dict__ARGS_DO_KEEP_PATHS} )
        set( OPTS ${OPTS} -noIncludePaths )
    endif()
    if( ${ROOT_dict__ARGS_INTERPRETER_ONLY} )
        set( OPTS ${OPTS} -interpreteronly )
    endif()

    add_custom_command( OUTPUT ${dictionary}.cxx
        COMMAND ${ROOTCLING_EXECUTABLE} -f ${dictionary}.cxx
            -c ${OTPS} ${includedirs_str} ${headerfiles} ${_LINKDEF_FILE}
        DEPENDS ${headerfiles} ${linkdefs} ${ROOT_dict__ARGS_TAGET_LIB}
        VERBATIM )
endfunction( )


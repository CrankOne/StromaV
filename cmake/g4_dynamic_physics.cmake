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

macro( collect_Geant4_definitions FILTERING_SCRIPT TEMPLATE_IN HEADER_OUT HDRPRFX )
    execute_process(COMMAND
        ${FILTERING_SCRIPT}
        ${Geant4_PHLIST_INCLUDE_DIR}
        OUTPUT_VARIABLE _DYNAMIC_ENTRIES )
    if( _DYNAMIC_ENTRIES )
        string(REPLACE "\n" ";" _DYNAMIC_ENTRIES ${_DYNAMIC_ENTRIES})
        #string(REPLACE ","  ";" _DYNAMIC_ENTRIES ${_DYNAMIC_ENTRIES} )
        list( REMOVE_DUPLICATES _DYNAMIC_ENTRIES )
        list( LENGTH _DYNAMIC_ENTRIES _DYNAMIC_ENTRIES_SIZE )
        get_filename_component( FLTRNGSCRPTNM ${FILTERING_SCRIPT} NAME )
        message( STATUS "Found ${_DYNAMIC_ENTRIES_SIZE} unique entries with \".../${FLTRNGSCRPTNM}\"." )
        foreach( entry ${_DYNAMIC_ENTRIES} )
            set( _INCLUDES_STR
                "${_INCLUDES_STR}\n# include <${HDRPRFX}${entry}.hh>" )
            set( _DEFINES_STR
                "${_DEFINES_STR}    m(${entry}) \\\n" )
        endforeach( entry )
        # TODO: conditionally determine if we really need to re-generate
        # it as now each `$ cmake' invokation causes rebuild.
        configure_file( ${TEMPLATE_IN} ${HEADER_OUT} @ONLY )
    else( _DYNAMIC_ENTRIES )
        message( SEND_ERROR "No dynamic entries found by ${FILTERING_SCRIPT}." )
    endif( _DYNAMIC_ENTRIES )
    unset( _DYNAMIC_ENTRIES )
    unset( _DYNAMIC_ENTRIES_SIZE )
    unset( _INCLUDES_STR )
    unset( _DEFINES_STR )
endmacro( collect_Geant4_definitions )


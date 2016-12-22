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

cmake_minimum_required( VERSION 2.6 )
project( afpipe )

include_directories( "${PROJECT_SOURCE_DIR}/inc/"
                     "${PROJECT_SOURCE_DIR}/../../inc/" )

file( GLOB_RECURSE afpipe-implem_SRCS 
    ${PROJECT_SOURCE_DIR}/src/*.c*)
list( REMOVE_ITEM afpipe-implem_SRCS 
    ${PROJECT_SOURCE_DIR}/src/main.cpp )

file( GLOB afpipe_SRCS ${PROJECT_SOURCE_DIR}/src/main.cpp )

set( afpipe_exec afpipe${StromaV_BUILD_POSTFIX}
    CACHE STRING "StromaV pipeline exec name." )
set( afpipe_implem afpipe_implem${StromaV_BUILD_POSTFIX}
    CACHE STRING "StromaV pipeline implementation library name." )

add_library( ${afpipe_implem} SHARED ${afpipe-implem_SRCS} )
target_link_libraries( ${afpipe_implem} ${StromaV_LIB} )

add_executable( ${afpipe_exec} ${afpipe_SRCS} )
target_link_libraries( ${afpipe_exec} ${afpipe_implem} )

install( TARGETS ${afpipe_exec}     RUNTIME DESTINATION bin )
install( TARGETS ${afpipe_implem}   LIBRARY DESTINATION lib/StromaV )


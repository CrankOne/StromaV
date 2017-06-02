# Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
# Author: Renat R. Dusaev <crank@qcrypt.org>
# Author: Bogdan Vasilishin <togetherwithra@gmail.com>
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
project( svmc )

include_directories( "${PROJECT_SOURCE_DIR}/inc/"
                     "${PROJECT_SOURCE_DIR}/../../inc/" )

file( GLOB_RECURSE svmc-implem_SRCS
    ${PROJECT_SOURCE_DIR}/src/*.c*)
list( REMOVE_ITEM svmc-implem_SRCS
    ${PROJECT_SOURCE_DIR}/src/main.cpp )

file( GLOB svmc_SRCS ${PROJECT_SOURCE_DIR}/src/main.cpp )

set( svmc_exec svmc${StromaV_BUILD_POSTFIX}
    CACHE STRING "StromaV svmc exec name." )
set( svmc_implem svmc_implem${StromaV_BUILD_POSTFIX}
    CACHE STRING "StromaV svmc implementation library name." )

add_library( ${svmc_implem} SHARED ${svmc-implem_SRCS} )
#target_link_libraries( ${svmc_implem} ${StromaV_LIB} )
target_link_libraries( ${svmc_implem} ${afNA64_LIB} )

add_executable( ${svmc_exec} ${svmc_SRCS} )
target_link_libraries( ${svmc_exec} ${svmc_implem} )

install( TARGETS ${svmc_implem}   EXPORT StromaV LIBRARY DESTINATION lib/na64 )
install( TARGETS ${svmc_exec}     EXPORT StromaV RUNTIME DESTINATION bin )


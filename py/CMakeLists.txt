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

cmake_minimum_required( VERSION 2.8.8 )
project( pyStromaV )

#
# Pre-set python versioning variables for proper look-up
# Note, that Python_ADDITIONAL_VERSIONS variable will be used for both py libs
# and interp. This way one can override default system's python version.
set( Python_ADDITIONAL_VERSIONS 2.7 2.6 2.5 2.4 )

find_package( SWIG REQUIRED )
find_package( PythonInterp REQUIRED )
find_package( PythonLibs REQUIRED )

include( ${SWIG_USE_FILE} )
set( CMAKE_SWIG_FLAGS "-Wall" )
include_directories( ${PYTHON_INCLUDE_DIRS} )

set_source_files_properties( analysis.i PROPERTIES CPLUSPLUS ON )
#set_property( SOURCE analysis.i PROPERTY SWIG_MODULE_NAME analysis )
#set_source_files_properties(analysis.i PROPERTIES SWIG_FLAGS "-includeall")
swig_add_module( analysis python analysis.i )
swig_link_libraries( analysis ${PYTHON_LIBRARIES})

execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print get_python_lib()"
                OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    set( PY_MODULES_INSTALL_DIR ${PYTHON_SITE_PACKAGES} )
else( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    # Don't know if it is legal:
    string(REPLACE "." ";" VERSION_LIST ${PYTHONLIBS_VERSION_STRING})
    list(GET VERSION_LIST 0 PYTHON_VERSION_MAJOR)
    list(GET VERSION_LIST 1 PYTHON_VERSION_MINOR)
    set(PY_VER "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")

    set( PY_MODULES_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/python${PY_VER}/site-packages )
    message( STATUS "Python site-packages dir is set to ${PY_MODULES_INSTALL_DIR}." )
endif( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )

target_link_libraries( _analysis ${StromaV_LIB} )

install(TARGETS _analysis
        DESTINATION ${PY_MODULES_INSTALL_DIR}/sV )
install(FILES ${CMAKE_BINARY_DIR}/py/analysis.py
        DESTINATION ${PY_MODULES_INSTALL_DIR}/sV )
# ... other generated wrappers
install( DIRECTORY . DESTINATION ${PY_MODULES_INSTALL_DIR}/sV FILES_MATCHING PATTERN
    "*.py" )

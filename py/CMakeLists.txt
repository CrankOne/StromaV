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

include(${Goo_CMAKE_MODULES_DIR}/PyExtensions.cmake)

set( SWIG_IFACES_DIR share/StromaV/swig-interfaces )

if( RPC_PROTOCOLS AND ANALYSIS_ROUTINES )
    list( APPEND StromaV_PYMODULES sVEvents.i appUtils.i pipeline.i gooDict.i
        extParameters.i )
    install( FILES
             # System types:
             std_unique_ptr.i
             std_stream.i
             # Common includes
             _commonProtobuf.i
             _gooExceptionWrapper.i
             # sV binding modules:
             appUtils.i
             sVEvents.i
             pipeline.i
             gooDict.i
             extParameters.i
             DESTINATION ${SWIG_IFACES_DIR}/sV  )
endif( RPC_PROTOCOLS AND ANALYSIS_ROUTINES )

if ( StromaV_PYMODULES )
    set( CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} -templatereduce )
    Goo_PY_EXTENSIONS( INTERFACES ${StromaV_PYMODULES}
                       LINK_LIBS ${StromaV_LIB}
                       WRAPPER_SOUCES WRAPPER_SRCS
                       SETUP_TEMPLATE setup.py.in
                       INIT_TEMPLATE __init__.py.in )
    #target_include_directories( _events
            #PUBLIC $<TARGET_PROPERTY:${StromaV_LIB},INCLUDE_DIRECTORIES>
        #)
    # Create symlink to make python's eggs mechanics happy:
    add_custom_target( py_link_target ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/py ${CMAKE_BINARY_DIR}/StromaV)

    # Note: workarounds for recent C++ compiler warnings
    set_source_files_properties( ${WRAPPER_SRCS}
            PROPERTIES COMPILE_FLAGS "-Dregister= -Wno-self-assign -Wno-shadow" )

    if( RPC_PROTOCOLS )
        protobuf_generate_python( PY_MSG_MODULES ../event.proto )
        add_custom_target( sVPyEvent ALL DEPENDS ${PY_MSG_MODULES} )
        #install( FILES ${PY_MSG_MODULES} DESTINATION ${PY_MODULES_INSTALL_DIR}/${PYPKGNM} )
    endif( RPC_PROTOCOLS )

    set( StromaV_SWIG_IFACES_DIR ${SWIG_IFACES_DIR} PARENT_SCOPE )
else( StromaV_PYMODULES )
    message( STATUS "No py-bindings to be generated" )
endif( StromaV_PYMODULES )


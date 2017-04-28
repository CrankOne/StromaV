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
project( P348CommandWidget )

include_directories( ${PROJECT_SOURCE_DIR} )

ROOT_GENERATE_DICTIONARY( P348CommandWidgetPluginDict
    ${PROJECT_SOURCE_DIR}/P348CommandWidgetPlugin.h*
    LINKDEF P348CommandWidgetPluginLinkDef.h )

file(GLOB_RECURSE P348CommandWidgetPlugin_SRCS ${PROJECT_SOURCE_DIR}/*.c* )

add_library( P348CommandWidgetPlugin SHARED ${P348CommandWidgetPlugin_SRCS}
             P348CommandWidgetPluginDict.cxx )

#
# Link the plugin against our lib
target_link_libraries( P348CommandWidgetPlugin ${StromaV_LIB} )
target_link_libraries( P348CommandWidgetPlugin ${ROOT_LIBRARIES} )

install( TARGETS P348CommandWidgetPlugin LIBRARY DESTINATION
    ${StromaV_ROOT_PLUGINS_INSTALL_PATH} )
install( FILES P348CommandWidgetPlugin.h
         DESTINATION include/StromaV/CommandWidget.root-plugin )
install( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/P348CommandWidgetPluginDict_rdict.pcm
    DESTINATION ${StromaV_ROOT_PLUGINS_INSTALL_PATH} )
#install( DIRECTORY inc/ DESTINATION include/StromaV/CommandWidget.root-plugin
#    FILES_MATCHING PATTERN "*.h"
#                   PATTERN "*.hpp"
#                   PATTERN "*.tcc"
#                   PATTERN "*.itcc" )

#configure_file (
#    "${PROJECT_SOURCE_DIR}/handler.C.in"
#    "${CMAKE_CURRENT_BINARY_DIR}/../TGCommandPlugin/P10_P348CommandWidgetPlugin.C"
#)

#add_custom_command(
#    TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ln -sf ${libname_major_minor_patch} ${libname_major_minor}
#    COMMAND ln -sf ${libname_major_minor_patch} ${libname_major}
#)


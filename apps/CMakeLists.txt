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
project( StromaVUtils )

#\option
push_option( build_pipeline
            "Build data pipelining library and executable."
            OFF )
#\option
push_option( build_mdlv
            "Build TGDML browsing library and executable."
            OFF )
#\option
push_option( build_unit_tests
            "Build unit tests for StromaV library."
            ON )

#\opt-dep:
option_depend( build_pipeline   ANALYSIS_ROUTINES RPC_PROTOCOLS )
#\opt-dep:
option_depend( build_mdlv       GEANT4_MC_MODEL G4_MDL_GUI G4_MDL_VIS )

if( build_pipeline )
    add_subdirectory( pipeline )
endif( build_pipeline )

if( build_mdlv )
    add_subdirectory( mdlv )
endif( build_mdlv )

if( build_unit_tests )
    add_subdirectory( ut )
endif( build_unit_tests )


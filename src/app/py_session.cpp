/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

# include "app/py_session.hpp"

# ifdef PYTHON_BINDINGS

namespace sV {

static const char _static_dftPySessionArgs[] = "sVPy -v3";

static goo::dict::Configuration * _static_dftPySessionConf = nullptr;

goo::dict::Configuration *
default_pythob_session_app_conf() {
    if( _static_dftPySessionConf ) {
        return _static_dftPySessionConf;
    }

    char ** argv;
    int argc = goo::dict::Configuration::tokenize_string( _static_dftPySessionArgs, argv );
    _static_dftPySessionConf = new goo::dict::Configuration( "sVPy",
        "StromaV python session configuration." );
    _static_dftPySessionConf->extract( argc, argv, true );

    // goo::dict::Configuration::free_tokens( argc, argv );

    return _static_dftPySessionConf;
}

}  // namespace sV

# endif


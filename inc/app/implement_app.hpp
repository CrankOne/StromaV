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

# ifndef H_STROMA_V_DEFAULT_APPLICATION_IMPL_H
# define H_STROMA_V_DEFAULT_APPLICATION_IMPL_H

# include "sV_config.h"
# include <cstdlib>
# include <boost/exception/diagnostic_information.hpp>

# define NOCATCH_ENVVAR "NOCATCH"

/**@brief Default application entry point implementation.
 *
 * This macro implements common scheme for implemented sV::Application
 * descendant.
 *
 * It is possible to disabe end-point exception handler by providing NOCATCH
 * environment variable set to '1', 'yes' or 'true'.
 *
 * FIXME: if variables_map is not allocated in heap, some application
 *        implementation causes it te be deleted twice...
 * */
# define StromaV_DEFAULT_APP_INSTANCE_ENTRY_POINT( appClass )                   \
int main( int argc, char * argv[] ) {                                           \
    ::goo::aux::iApp::add_environment_variable(                                 \
        NOCATCH_ENVVAR,                                                         \
        "Whether to catch exceptions at the outermost context."                 \
    );                                                                          \
    if( ::goo::aux::iApp::envvar_as_logical( NOCATCH_ENVVAR ) ) {               \
        sV::po::variables_map * vmPtr = new sV::po::variables_map();            \
        sV::AbstractApplication::init(argc, argv, new appClass(vmPtr) );        \
        return sV::AbstractApplication::run();                                  \
    } else {                                                                    \
        try {                                                                   \
            sV::po::variables_map * vmPtr = new sV::po::variables_map();        \
            sV::AbstractApplication::init(argc, argv, new appClass(vmPtr) );    \
            return sV::AbstractApplication::run();                              \
        } catch( goo::Exception & e ) {                                         \
            std::cerr << "Caught an instance of goo::Exception:" << std::endl;  \
            e.dump(std::cerr);                                                  \
            std::cerr << "Aborting." << std::endl;                              \
        } catch( boost::exception & e ) {                                       \
            std::cerr << "Caught an instance of boost::exception: " << std::endl \
                      /*<< e.what()*/                                           \
                      /*<< boost::trace(e)  // TODO: they're still developing this...*/ \
                      << boost::diagnostic_information(e)                       \
                      << std::endl;                                             \
        } catch( std::exception & e ) {                                         \
            std::cerr << "Caught an instance of std::exception: "               \
                      << e.what() << std::endl;                                 \
            std::cerr << "Aborting." << std::endl;                              \
        } catch( std::string & str ) {                                          \
            std::cerr << "Caught std::string: \"" << str << "\". Aborting." << std::endl; \
        } catch( const char * cstr ) {                                          \
            std::cerr << "Caught C-string: \"" << cstr << "\". Aborting." << std::endl; \
        } catch( ... ) {                                                        \
            std::cerr << "Caught an unknown exception. Aborting." << std::endl; \
        }                                                                       \
        return EXIT_FAILURE;                                                    \
    }                                                                           \
}
# endif  /* H_STROMA_V_DEFAULT_APPLICATION_IMPL_H */


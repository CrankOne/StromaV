// This is a helper file for shared_ptr and should not be included directly.
// Implementation proposed by Flexo at StackOverflow:
//          http://stackoverflow.com/a/18932980/1734499

%{
# include <stdio.h>
# include <boost/iostreams/stream.hpp>
# include <boost/iostreams/device/file_descriptor.hpp>

namespace io = boost::iostreams;
typedef io::stream_buffer<io::file_descriptor_sink> boost_ofdstream;
typedef io::stream_buffer<io::file_descriptor_source> boost_ifdstream;

struct in_StreamWrappingBundle {
    boost_ifdstream * inFdstreamPtr;
    std::shared_ptr<std::istream> inStreamShrdPtr;
};
%}

//
// Ordinary i/ostream
////////////////////

# if 1
%typemap(in) std::ostream& (boost_ofdstream *stream=NULL) {
    FILE * f = PyFile_AsFile($input); // Verify the semantics of this
    if( !f ) {
        SWIG_Error(SWIG_TypeError, "File object expected.");
        SWIG_fail;
    } else {
        // If threaded incrment the use count
        stream = new boost_ofdstream(fileno(f), io::never_close_handle);
        $1 = new std::ostream(stream);
  }
}

%typemap(in) std::istream& (boost_ifdstream *stream=NULL) {
    FILE * f = PyFile_AsFile($input);
    // Verify that this returns NULL for non-files
    if( !f ) {
        SWIG_Error(SWIG_TypeError, "File object expected.");  
        SWIG_fail;
    } else {
        stream = new boost_ifdstream(fileno(f), io::never_close_handle);
        $1 = new std::istream(stream);
    }
}

%typemap(freearg) std::ostream& {
    delete $1;
    delete stream$argnum;
}

%typemap(freearg) std::istream& {
    delete $1;
    delete stream$argnum;
}
# endif

//
// Shared ptr
////////////

%typemap(in) std::shared_ptr<std::istream> & (in_StreamWrappingBundle * streamBundle=NULL) {
    FILE * f = PyFile_AsFile( $input );
    // Verify that this returns NULL for non-files
    if( !f ) {
        SWIG_Error(SWIG_TypeError, "File object expected.");  
        SWIG_fail;
    } else {
        streamBundle = new in_StreamWrappingBundle;
        streamBundle->inFdstreamPtr
                    = new boost_ifdstream(fileno(f),
                                          io::never_close_handle);
        streamBundle->inStreamShrdPtr.reset( new std::istream(
                                                streamBundle->inFdstreamPtr ),
                                             [$input, streamBundle](std::istream * p){
                PyFile_DecUseCount( (PyFileObject *) $input );
                delete p;
            } );
        $1 = &(streamBundle->inStreamShrdPtr);
        PyFile_IncUseCount( (PyFileObject *) $input );
    }
}

%typemap(freearg) std::shared_ptr<std::istream> & {
    delete streamBundle$argnum;
}



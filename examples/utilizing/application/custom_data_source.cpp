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

/**@file data_source
 * @brief An example that demonstrates how to implement a
 * custom data source for analysis application.
 *
 * 
 */

// This is for application routines:
# include "app/analysis.hpp"
// This refers to universal event
# include "p348g4_uevent.hpp"

namespace p348 {
namespace dsources {
namespace example {

class MyFile : public AnalysisApplication::iEventSequence {
public:
    typedef AnalysisApplication::iEventSequence Parent;
    typedef typename AnalysisApplication::Event Event;
private:
    // ... private members --- data acquizition parameters,
    // ... managers, network settings, etc. ...
protected:
    // initializes event reading: open file/socket, set counters, etc.
    virtual Event * _V_initialize_reading() override;
    // should return true, if there are events available
    virtual bool _V_is_good() override;
    // should set the new event by provided pointer reference
    virtual void _V_next_event( Event *& ) override;
    // should free file/socket descriptor, memory, perform cleanup, etc.
    virtual void _V_finalize_reading() override;
    // prints brief summary of events read, failures, errors, etc.
    virtual void _V_print_brief_summary( std::ostream & ) const override;
public:
    MyFile( /* constructor parameters here */ ) { /* ... */ }
    virtual ~MyFile() { /* ... */ }

    // ...
};

MyFile::Event *
MyFile::_V_initialize_reading() {
    // ...
    // Here one should:
    // - Open file/socket/etc.
    // - Allocate any auxilliary data.
    // - Allocate and return reentrant event object and
    // return its pointer.
    return nullptr;  // TODO
}

bool
MyFile::_V_is_good() {
    // ...
    // Here one should:
    // - return «true» only when further reading of event is possible.
    // When there are no more events in source, this method should
    // return «false»;
    return false;  // TODO
}

void
MyFile::_V_next_event( MyFile::Event *& ) {
    // ...
    // Here one should:
    // - write event information into provided
    // reentrant pointer object. Note, that single argument is the
    // reference to pointer and changing it is valid by external
    // routines. For some data sources such ability can be useful.
}

void
MyFile::_V_finalize_reading() {
    // ...
    // Here one should:
    // - close file/socket/etc.
    // - clean-up any auxilliary data.
}

void
MyFile::_V_print_brief_summary( std::ostream & ) const {
    // ...
    // Here one may print out any meaning information about
    // data source --- e.g. events in queue, resources usage,
    // descriptors info, etc. Mainly used for debug.
}

//
// MyFile insertion routines
//

p348_DEFINE_CONFIG_ARGUMENTS {
    po::options_description myFileCfg( "DaqDataDecoding library data source options" );
    { myFileCfg.add_options()
        ("myFile.someparameter",
            po::value<std::string>(),
            "Here be some parameter according to boost::variables_map::options_description " )
        ("myFile.other-parameter",
            po::value<bool>()->default_value(false),
            "Some other parameter.")
        // ...
        ;
    }
    return myFileCfg;
}

p348_DEFINE_DATA_SOURCE_FMT_CONSTRUCTOR( File ) {
    return new MyFile( /* constructor parameters here */ );
}
p348_REGISTER_DATA_SOURCE_FMT_CONSTRUCTOR( File, "myFile", "Example events data source." )


}  // namespace example
}  // namespace dsources
}  // namespace p348



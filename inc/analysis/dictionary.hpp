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

# ifndef H_STROMA_V_ANALYSIS_DICTIONARY_H
# define H_STROMA_V_ANALYSIS_DICTIONARY_H

# include "../config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/pipeline.hpp"
# include "app/abstract.hpp"

namespace sV {

/**@class AnalysisDictionary
 * @brief Dictionary indexing available data analysis routines.
 *
 * This dictionary is designed for automated run-time indexing of all data
 * sources and data processing handlers. This class implements no actual
 * object and represents more like a scope summing up the static indexing
 * maps and access functions.
 * */
class AnalysisDictionary {
public:
    typedef po::options_description (*OptionsSupplement)();

    typedef AnalysisPipeline::iEventSequence iEventSequence;
    typedef AnalysisPipeline::iEventProcessor iEventProcessor;
    /// Data source constructor type.
    typedef iEventSequence *(*EvSeqCtr)();
    /// Handler constructor type.
    typedef iEventProcessor *(*EvProcCtr)();
protected:
    /// Set of named reading object constructors.
    static std::unordered_map<
                std::string,
                std::pair<EvSeqCtr, std::string> >  * _readersDict;
    /// Set of named handlers constructors.
    static std::unordered_map<
                std::string,
                std::pair<EvProcCtr, std::string> > * _procsDict;
    /// Supplementary options usually filled by processors.
    static std::unordered_set<OptionsSupplement> * _suppOpts;
public:
    /// Inserts reader callback.
    static void add_reader( const std::string &, EvSeqCtr,
                            const std::string & descr );
    /// Prints out available reader callbacks keys.
    static void list_readers( std::ostream & );
    /// Looks for reader callback pointed out by string. Raises notFound on
    /// failure.
    static EvSeqCtr find_reader( const std::string & );
    /// Inserts processor callback.
    static void add_processor( const std::string &, EvProcCtr,
                               const std::string & descr );
    /// Prints out available processors keys.
    static void list_processors( std::ostream & );
    /// Looks for processor pointed out by string. Raises notFound on failure.
    static EvProcCtr find_processor( const std::string & );
    /// Adds options supplement routine.
    static void supp_options( OptionsSupplement );
};  // class AnalysisDictionary

/**@def StromaV_DEFINE_DATA_SOURCE_FMT_CONSTRUCTOR
 * @brief A data format reader constructor implementation macro. */
# define StromaV_DEFINE_DATA_SOURCE_FMT_CONSTRUCTOR( ReaderClass )           \
static void __static_register_ ## ReaderClass ## _fmt() __attribute__ ((constructor(156))); \
static sV::AnalysisDictionary::iEventSequence * _static_construct_ ## ReaderClass ()

/**@def StromaV_REGISTER_DATA_SOURCE_FMT_CONSTRUCTOR
 * @brief A data format reader constructor insertion macro. */
# define StromaV_REGISTER_DATA_SOURCE_FMT_CONSTRUCTOR( ReaderClass, txtName, descr ) \
static void __static_register_ ## ReaderClass ## _fmt() {                   \
    sV::AnalysisDictionary::add_reader( \
                        txtName, _static_construct_ ## ReaderClass, descr ); }

/**@def StromaV_DEFINE_DATA_PROCESSOR
 * @brief A data processing constructor implementation macro. */
# define StromaV_DEFINE_DATA_PROCESSOR( ProcessorClass )                     \
static void __static_register_ ## ProcessorClass ## _prc() __attribute__ ((constructor(156)));  \
static sV::AnalysisDictionary::iEventProcessor * _static_construct_ ## ProcessorClass ()

/**@def StromaV_DEFINE_CONFIG_ARGUMENTS
 * @brief Supplementary configuration insertion macro for \ref AnalysisDictionary . */
# define StromaV_DEFINE_CONFIG_ARGUMENTS                                     \
static ::boost::program_options::options_description _get_supp_options();   \
static void __static_register_args() __attribute__ ((constructor(156)));    \
static void __static_register_args() {                                      \
    sV::AnalysisDictionary::supp_options( _get_supp_options );}         \
static ::boost::program_options::options_description _get_supp_options()

/**@def StromaV_REGISTER_DATA_SOURCE_FMT_CONSTRUCTOR
 * @brief A data processor constructor insertion macro. */
# define StromaV_REGISTER_DATA_PROCESSOR( ProcessorClass, txtName, descr )   \
static void __static_register_ ## ProcessorClass ## _prc() {                \
    sV::AnalysisDictionary::add_processor( txtName, _static_construct_ ## ProcessorClass, descr ); }

}  // namespace sV

# endif  // RPC_PROTOCOLS
# endif  // H_STROMA_V_ANALYSIS_DICTIONARY_H


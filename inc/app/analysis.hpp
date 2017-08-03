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

# ifndef H_STROMA_V_ANALYSIS_APPLICATION_BASE_H
# define H_STROMA_V_ANALYSIS_APPLICATION_BASE_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "app/abstract.hpp"
# include "app/mixins/protobuf.hpp"
# include "app/mixins/root.hpp"
# include "analysis/pipeline.hpp"

namespace sV {

/**@class AnalysisApplication
 * @brief Base class for generic-purpose data analysis.
 *
 * This mixin implements base for event-by-event processing applications.
 * The pipeline approach implies a series of handlers (here called
 * "processors") that implement subsequent treatment of data provided in
 * particular event.
 *
 * The common idea of this kind of applications implies sequential reading
 * data from files (event-by-event). Variable parts of them:
 *      - Initial application configuration abstraction; implemented here with
 *      boost::variables_map.
 *      - The data source; we have here at least three types of data formats:
 *          * a ROOT tree
 *          * a "raw data" that has to be decoded by DaqDataDecoding
 *            meanings.
 *          * a compressed universal event format implemented via GPB.
 *      - The postprocessing routine that gathers walkthrough information from
 *      events array.
 *      - The postprocessing routine (including possible output to data streams
 *      or files).
 *
 * This class implements some common basics for configuration and such a sequential
 * reading in order to consuming descendants elaborate its routines.
 *
 * @ingroup app
 * @ingroup analysis
 */
class AnalysisApplication :
        public mixins::PBEventApp,
        public mixins::RootApplication,
        public AnalysisPipeline,
        public virtual sV::AbstractApplication {
private:
    /// Pointer to data source.
    iEventSequence * _evSeq;
public:
    typedef AbstractApplication Parent;
    typedef typename mixins::PBEventApp::UniEvent Event;

    /// Concrete config object type shortcut (variables map from boost lib).
    typedef Parent::Config Config;
protected:
    // INTERFACE
    /// Called after common configuration options is done. Can set 
    /// _immediateExit flag.
    virtual void _V_concrete_app_configure() override;
    /// Appends updating of ASCII display upon successfull finish of single
    /// event processing.
    virtual void _finalize_event( Event * ) override;

    AnalysisApplication( Parent::Config * vm );
public:
    virtual ~AnalysisApplication();

    /// Current event sequence getter.
    iEventSequence & event_sequence() {
        const AnalysisApplication * cThis = this;
        return const_cast<iEventSequence &>( cThis->event_sequence() );
    }

    /// Returns reference to an event sequence set.
    const iEventSequence & event_sequence() const {
        if( !event_sequence_set() ) {
            emraise( badState, "Event sequence object is not set for pipeline "
                    "%p.", this )
        }
        return *_evSeq;
    }

    /// Returns whether the event sequence had been set.
    bool event_sequence_set() const { return !!_evSeq; }

    /// Sets the event sequence for the pipeline.
    // TODO: void event_sequence( iEventSequence * ) const;

    /// Find processor by name and push back it to chain.
    //void push_back_processor( const std::string & );
};  // class AnalysisApplication

}  // namespace sV

/// Shortcut for define virtual ctr for event processing classes without common
/// config mapping.
# define StromaV_ANALYSIS_PROCESSOR_DEFINE( cxxClassName,                   \
                                            name )                          \
StromaV_DEFINE_STD_CONSTRUCTABLE( cxxClassName, name, sV::aux::iEventProcessor )


/// Shortcut for define virtual ctr for event processing classes with common
/// config mapping.
# define StromaV_ANALYSIS_PROCESSOR_DEFINE_MCONF( cxxClassName,             \
                                                  name )                    \
StromaV_DEFINE_STD_CONSTRUCTABLE_MCONF( cxxClassName, name, sV::aux::iEventProcessor )


/// Shortcut for define virtual ctr for event reading classes without common
/// config mapping.
# define StromaV_EVENTS_SEQUENCE_DEFINE( cxxClassName,                      \
                                            name )                          \
StromaV_DEFINE_STD_CONSTRUCTABLE( cxxClassName, name, sV::aux::iEventSequence )


/// Shortcut for define virtual ctr for event reading classes with common
/// config mapping.
# define StromaV_EVENTS_SEQUENCE_DEFINE_MCONF( cxxClassName,                \
                                                  name )                    \
StromaV_DEFINE_STD_CONSTRUCTABLE_MCONF( cxxClassName, name, sV::aux::iEventSequence )

# endif  // RPC_PROTOCOLS
# endif  // H_STROMA_V_ANALYSIS_APPLICATION_BASE_H


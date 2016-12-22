%module analysis
%{

#include "analysis/pipeline.hpp"
#if !defined( RPC_PROTOCOLS ) || !defined( ANALYSIS_ROUTINES )
#error "Either RPC_PROTOCOLS or ANALYSIS_ROUTINES not defined. Unable to " \
"build analysis module."
#endif

#include "analysis/dictionary.hpp"
#include "analysis/processors/testProc.hpp"

typedef sV::AnalysisPipeline::iEventProcessor iEventProcessor;

%}

%include "config.h"
%include "analysis/pipeline.hpp"
%include "analysis/ra_event_source.itcc"
%include "analysis/dictionary.hpp"
%include "analysis/processors/testProc.hpp"


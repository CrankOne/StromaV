%module pipeline

%ignore PACKAGE_VERSION;

/* SWIG of versions at least >=2.0.9 doesn't like the C++11 override/final
 * keywords, so we get rid of them using these macro defs: */
# ifdef SWIG
# define override
# define final
# endif  // SWIG

%{

# include "config.h"

#if !defined( RPC_PROTOCOLS )
#error "RPC_PROTOCOLS is not " \
"defined. Unable to build pipeline py-wrapper module."
#endif

#include "analysis/pipeline.hpp"
#if !defined( RPC_PROTOCOLS ) || !defined( ANALYSIS_ROUTINES )
#error "Either RPC_PROTOCOLS or ANALYSIS_ROUTINES not defined. Unable to " \
"build analysis module."
#endif

#include "analysis/dictionary.hpp"

%}

%include "config.h"
%include "analysis/pipeline.hpp"
%include "analysis/ra_event_source.itcc"
%include "analysis/dictionary.hpp"


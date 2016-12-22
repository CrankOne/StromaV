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

# ifndef H_STROMA_V_PROTOCOL_BUFFER_UNIFIED_EVENT_APPLICATION_MIXIN_H
# define H_STROMA_V_PROTOCOL_BUFFER_UNIFIED_EVENT_APPLICATION_MIXIN_H

# include "../../config.h"

# ifdef RPC_PROTOCOLS

# include "app/abstract.hpp"

namespace sV {

namespace events {
class Event;
class ExperimentalEvent;
class SADC_profile;
class APV_sampleSet;
}  // namespace events

namespace mixins {

/**@class PBEventApp
 *
 * @brief An application mixing providing centralized access to current event
 * in universal format.
 *
 * This class implies existance of data chunk (event) stored in unified format.
 * All the users can gain read or write access to this chunk. Useful for
 * implementing analysis pipelines (see sV::AnalysisApplication).
 *
 * @ingroup app
 */
class PBEventApp : public virtual AbstractApplication {
public:
    /// Unified event type.
    typedef sV::events::Event UniEvent;
private:
    /// Self reference.
    static PBEventApp * _self_PBEventAppPtr;
protected:
    /// Pointer to current event.
    UniEvent * _cEvent;
public:
    PBEventApp( AbstractApplication::Config * c );

    /// Returns current event.
    static UniEvent & c_event();
};

}  // namespace mixins

}  // namespace sV

# endif  // RPC_PROTOCOLS
# endif  // H_STROMA_V_PROTOCOL_BUFFER_UNIFIED_EVENT_APPLICATION_MIXIN_H


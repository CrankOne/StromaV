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

# ifndef H_STROMA_V_RANDOM_ACCESS_EVENT_SOURCE_H
# define H_STROMA_V_RANDOM_ACCESS_EVENT_SOURCE_H

# include "pipeline.hpp"
# include "metadata/type.tcc"

namespace sV {

namespace aux {

/**@class iRandomAccessEventSource
 * @brief An interim interface providing random access mthods.
 *
 * User code has to define particular metadata application routines.
 * */
template<typename EventIDT,
         typename SpecificMetadataT>
class iRandomAccessEventSource {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;  // TODO: reserve for future?
protected:
    /// Random access read event based on provided metadata information (IF).
    virtual bool _V_event_read_single( const EventIDT & ) = 0;

    /// Read events in some ID range (IF).
    virtual bool _V_event_read_range( const EventIDT & lower,
                                      const EventIDT & upper ) = 0;
    /// Read events specified by set of indexes (IF with default
    /// implementation).
    virtual bool _V_event_read_list( const std::list<EventID> & list ) {
        bool res = true;
        for( auto id : list ) {
            res &= event_read_single( id );
        }
        return res;
    }
public:
    iRandomAccessEventSource() {}

    virtual ~iRandomAccessEventSource() {}
    virtual bool event_read_single( const EventID & eid ) {
        return _V_event_read_single(eid); }

    virtual bool event_read_range( const EventID & lower,
                                   const EventID & upper ) {
        return _V_event_read_range( lower, upper ); }

    virtual bool event_read_list( const std::list<EventID> & list ) {
        return _V_event_read_list(list); }
};  // iRandomAccessEventSource

}  // namespace aux

}  // namespace sV

# endif  // H_STROMA_V_RANDOM_ACCESS_EVENT_SOURCE_H


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
# include "identifiable_ev_source.tcc"
# include "metadata_store.tcc"

namespace sV {

namespace aux {

/**@class iRandomAccessEventSource
 * @brief An interim interface providing random access mthods.
 *
 * User code has to define particular metadata application routines.
 * */
template<typename EventIDT,
         typename SpecificMetadataT>
class iRandomAccessEventSource : public iEventSequence {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef iEventSequence Parent;
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

    iRandomAccessEventSource( Parent::Features_t fts ) :
                        Parent( fts | iEventSequence::randomAccess ) {}
    iRandomAccessEventSource( ) : iRandomAccessEventSource( 0x0 ) {}
public:
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

/**@class iBatchEventSource
 * @brief Interface class for unique event source supporting metadata for
 *        random access.
 * 
 * Template class for event sequences supporting random access. All the 
 * `event_read_*` functions can return false upon non-critical failure as well
 * as indicate a critical error by throwing an exception.
 *
 * This class is designed as an intermediate representation of somewhat ideal
 * "unique" event source that supports metadata indexing on its own scope. It
 * can be further used as a container or manager dispatching queries among
 * multiple minor event source instances implemented by its sibling ---
 * iEventSource which provides additional identification mechanics for
 * multiple source instances of the same type.
 *
 * Adjoint metadata template base class is called iMetadataType.
 * */
template<typename EventIDT,
         typename SpecificMetadataT>
class iBatchEventSource : public aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT> {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef iMetadataType<EventID, SpecificMetadata> iSpecificMetadataType;
    typedef aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT> Parent;
private:
    SpecificMetadata * _raMDatCache;
protected:

    iBatchEventSource( aux::iEventSequence::Features_t fts ) :
                        Parent( fts ),
                        _raMDatCache(nullptr) {}
    iBatchEventSource( ) : iBatchEventSource( 0x0 ) {}
public:

    virtual ~iBatchEventSource() {
        if( _raMDatCache ) {
            delete _raMDatCache;
        }
    }

    /// Obtain (fetch cached or build new) metadata for itself.
    virtual const SpecificMetadata & metadata(
                                sV::MetadataDictionary<EventIDT> & mDict ) {
        if( !_raMDatCache ) {
            const iSpecificMetadataType & mdt = 
                        mDict.template get_metadata_type<SpecificMetadata>();
            _raMDatCache = &( mdt.acquire_metadata( *this ) );
        }
        return *_raMDatCache;
    }
};  // class iBatchEventSource


/**@class iSectionalEventSource
 * @brief Interface base class for identifiable event sources.
 *
 * This template interface is designed for description of multiple sources with
 * unique content usually corresponding to artifacts. For instance the
 * experimental statistics are often distributed among multiple homogenious
 * files. Each file might be represented by instance of descendant class.
 *
 * Such artifacts usually supports random access at their own scope.
 * For non-sectional data sources with random access but without internal
 * sectioning, the similar template class called
 * iBatchEventSource<...> was designed.
 *
 * Adjoint metadata template base class is called iCachedMetadataType.
 * */
template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
class iSectionalEventSource :
            public aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT>,
            public mixins::iIdentifiableEventSource<SourceIDT> {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef iCachedMetadataType<EventID, SpecificMetadata, SourceIDT>
            iSpecificMetadataType;
    typedef aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT>
            Parent;
private:
    SpecificMetadata * _mDatCache;
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
    iSectionalEventSource( aux::iEventSequence::Features_t fts ) :
                     Parent( fts | aux::iEventSequence::identifiable ),
                     _mDatCache(nullptr) {}

    iSectionalEventSource( ) :
                     iSectionalEventSource( 0x0 ) {}

    virtual ~iSectionalEventSource() {
        if( _mDatCache ) {
            delete _mDatCache;
        }
    }

    virtual bool event_read_single( const EventID & eid ) {
        return _V_event_read_single(eid); }

    virtual bool event_read_range( const EventID & lower,
                                   const EventID & upper ) {
        return _V_event_read_range( lower, upper ); }

    virtual bool event_read_list( const std::list<EventID> & list ) {
        return _V_event_read_list(list); }

    virtual const SpecificMetadata & metadata(
                        sV::MetadataDictionary<EventIDT> & mDict ) {
        if( !_mDatCache ) {
            const iSpecificMetadataType & mdt =
                    static_cast<iSpecificMetadataType> (
                    mDict.template get_metadata_type<SpecificMetadata>() );
            _mDatCache = &( mdt.acquire_metadata_for( *this ) );
        }
        return *_mDatCache;
    }
};  // class iSectionalEventSource

}  // namespace sV

# endif  // H_STROMA_V_RANDOM_ACCESS_EVENT_SOURCE_H


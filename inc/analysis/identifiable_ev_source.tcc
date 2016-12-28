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

# ifndef H_STROMA_V_IDENTIFIABLE_EVENT_SOURCE_H
# define H_STROMA_V_IDENTIFIABLE_EVENT_SOURCE_H

# include <goo_exception.hpp>

namespace sV {

namespace aux {
template<typename SourceIDT>
class iSourceIDParser {
public:
    typedef SourceIDT SourceID;
protected:
    SourceID _V_parse( const std::string & ) const = 0;
public:
    SourceID parse( const std::string & idStr ) const
        { return _V_parse(idStr); }
};  // class iSourceIDParser

}  // namespace aux

namespace mixins {

/**@class iIdentifiableEventSource
 * @brief Class representing event source trait that can be uniquely identified
 *        by value of some type.
 *
 * It does matter to know about identifiaction of event sources from metadata
 * aspect. In usual applications iser can identify event sources as some input
 * parameter (e.g. socket or filename).
 *
 * However, if data are distributed among multiple sources there appears a need
 * for indexing sources. Especially it reveals as a neccessary aspect when
 * metadata look-up comes to the stage.
 *
 * Since the conversion between ID and string is necessary for generating
 * human-readable messages and can be considered as a useful aspect at some
 * applications there is also an interface methods for printing and parsing
 * IDs.
 * */
template<typename SourceIDT>
class iIdentifiableEventSource {
public:
    typedef SourceIDT SourceID;
    typedef aux::iSourceIDParser<SourceID> SourceIDParser;
public:
    static SourceIDParser * IDParser;
private:
    bool _isIDInitialized;
    SourceID _id;
protected:
    /// (IF) Has to return verbose string ID identifier (can be NULL).
    virtual char * _V_textual_id( const SourceID * ) const = 0;
public:
    /// Default ctr for instances with uninitialized ID.
    iIdentifiableEventSource() : _isIDInitialized(false) {}

    /// Ctr. Initializes ID attr.
    iIdentifiableEventSource( SourceID & id ) : _isIDInitialized(true),
                                                _id(id) {}

    /// Ctr can be invoked only for template instances (classes) that have
    /// IDParer static attribute set. Throws goo::Exception/badState if parser
    /// was not set.
    iIdentifiableEventSource( const std::string & idStr ) :
                _isIDInitialized(true),
                _id(parse_id(idStr)) {}

    virtual ~iIdentifiableEventSource(){}

    /// Whether ID was initialized.
    virtual bool is_id_initialized() const { return _isIDInitialized; }

    /// Returns pointer to ID or NULL if it wasn't initialized.
    virtual const SourceID * id_ptr() const {
        return is_id_initialized() ? &_id : nullptr;
    }

    /// Returns verbose string ID identifier.
    virtual const char * textual_id() const
        { return _V_textual_id( id_ptr() ); }

    /// Shortcut to ID parser invokation.
    /// Throws goo::Exception/badState if parser was not set.
    static SourceID parse_id( const std::string & idStr ) {
        if( !IDParser ) {
            emraise( badState, "Source ID parser is not set for "
                "this type." );
        }
        return IDParser( idStr );
    }
};  // class iIdentifiableEventSource

template<typename SourceIDT>
aux::iSourceIDParser<SourceIDT> * 
                    iIdentifiableEventSource<SourceIDT>::IDParser = nullptr;

}  // namespace mixins

}  // namespace sV

# endif  // H_STROMA_V_IDENTIFIABLE_EVENT_SOURCE_H


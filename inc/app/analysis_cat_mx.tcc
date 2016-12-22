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

# ifndef H_STROMA_V_TFILE_TDIRECTORY_DISPATCHER_MIXIN_H
# define H_STROMA_V_TFILE_TDIRECTORY_DISPATCHER_MIXIN_H

# include <TDirectory.h>
# include <TFile.h>
# include <unordered_map>
# include <goo_exception.hpp>
# include <iostream>  // XXX
# include "../detector_ids.h"

namespace sV {
namespace mixins {

/**@class ThematicDirectoryStructure
 * @brief A helper mixin to order TFile structure into directories.
 *
 * It is frequent task to arrange some data into folders inside a TFile
 * according to categories (detector, trigger conditions, etc). The
 * actual indexing mechanism is incapsulated at interface functions and
 * hashable types.
 */
template<typename HashKeyT,
         typename CategoryKeyT,
         typename CategorizedEntryT>
class ThematicDirectoryStructure {
public:
    typedef HashKeyT HashKey;
    typedef CategoryKeyT CategoryKey;
    typedef CategorizedEntryT CategorizedEntry;
protected:
    const std::string _rootDirectoryName;

    TDirectory * _tCategoryDir;
    std::unordered_map<CategoryKey, TDirectory *> _subdirs;
    std::unordered_map<HashKey, CategorizedEntry> _entries;

    virtual CategorizedEntry _V_new_entry(HashKey, TDirectory *) = 0;
    virtual CategoryKey _V_get_category_key(HashKey) const = 0;
    virtual const char * _V_get_category_name( CategoryKey ) const = 0;
    virtual void _V_free_entry(HashKey, CategorizedEntry) = 0;
    virtual void _free_entries();
public:
    /// One must specify non-empty root category name.
    ThematicDirectoryStructure( const std::string & );
    /// Invokes _V_free_entry() in hashing order for each created entry.
    virtual ~ThematicDirectoryStructure() {}

    /// Returns true, if structure is initialized (root TDirectory created).
    virtual bool do_collect_stats() const { return _tCategoryDir; }
    /// Returns pointer to indexed entry.
    virtual CategorizedEntry & consider_entry( HashKey );
    /// Shortcut operator for \ref consider_entry()
    virtual CategorizedEntry & operator[]( HashKey k ) { return consider_entry(k); }
};  // class ThematicDirectoryStructure

//
// IMPLEMENTATION
//

template<typename HashKeyT,
         typename CategoryKeyT,
         typename CategorizedEntryT>
ThematicDirectoryStructure<HashKeyT, CategoryKeyT, CategorizedEntryT>::ThematicDirectoryStructure(
                        const std::string & directoryName ) :
                                _rootDirectoryName(directoryName),
                                _tCategoryDir(nullptr) {}

template<typename HashKeyT,
         typename CategoryKeyT,
         typename CategorizedEntryT> void
ThematicDirectoryStructure<HashKeyT, CategoryKeyT, CategorizedEntryT>::_free_entries() {
    for( auto it = _entries.begin(); it != _entries.end(); ++it ) {
        _V_free_entry( it->first, it->second );
    }
}

template<typename HashKeyT,
         typename CategoryKeyT,
         typename CategorizedEntryT>
CategorizedEntryT &
ThematicDirectoryStructure<HashKeyT, CategoryKeyT, CategorizedEntryT>::consider_entry( HashKey k ) {
    // try to find stats:
    auto it = _entries.find(k);
    // if not found, do:
    if( _entries.end() == it ) {
        // Try to found category directory:
        auto dirKey = _V_get_category_key( k );
        auto itDir = _subdirs.find( dirKey );
        // if directory not found, create it inside our local root directory:
        if( _subdirs.end() == itDir ) {
            # if 0
            std::cout << "Directory doesn't exist: "
                      << _V_get_category_name(dirKey)
                      << "(" << (int) k << ":" << (int) dirKey << ")"
                      << std::endl;
            # endif
            // check, if local root actually exists, and create it if not:
            if( !_tCategoryDir ) {
                if( gFile && !_rootDirectoryName.empty() ) {
                    gFile->cd();
                    _tCategoryDir = gFile->mkdir( _rootDirectoryName.c_str() );
                    # if 0
                    std::cout << "Root directory \"" << _rootDirectoryName << "\" created." << std::endl;
                    # endif
                } else {
                    emraise( badState, "Couldn't operate with directory \"%s\" since this name is empty, "
                    "or no ROOT file is opened.", _rootDirectoryName.c_str() );
                }
            }
            // create and register family directory:
            auto insertionResut = _subdirs.emplace(
                    dirKey,
                    _tCategoryDir->mkdir( _V_get_category_name( dirKey ) )
                );
            if( !insertionResut.second ) {
                emraise( malformedArguments, "Couldn't create TDirectory \"%s\" inside \"%s\".",
                            _V_get_category_name( dirKey ),
                            _rootDirectoryName.c_str()
                        );
            }
            # if 0
            std::cout << "Directory created: "
                      << "(" << (int) k << ":" << (int) dirKey << ")"
                      << _V_get_category_name(dirKey)
                      << std::endl;
            # endif
            itDir = insertionResut.first;
        }
        (*itDir).second->cd();
        auto insertionResult = _entries.emplace( k, _V_new_entry(k, (*itDir).second) );
        if( !insertionResult.second ) {
            emraise( nonUniq, "Couldn't emplace entry.");
        }
        it = insertionResult.first;
    }
    return it->second;
}

//
//
//

template<typename CategorizedEntryT>
class DetectorCatalogue : public ThematicDirectoryStructure<
                AFR_DetSignature,
                AFR_DetFamID,
                CategorizedEntryT> {
public:
    typedef ThematicDirectoryStructure<
                AFR_DetSignature,
                AFR_DetFamID,
                CategorizedEntryT> Parent;
protected:
    virtual AFR_DetFamID _V_get_category_key( AFR_DetSignature ) const;
    virtual const char * _V_get_category_name( AFR_DetFamID ) const;
public:
    DetectorCatalogue( const std::string & catName ) :
                ThematicDirectoryStructure<
                    AFR_DetSignature,
                    AFR_DetFamID,
                    CategorizedEntryT>( catName ) {}
};  // class DetectorCatalogue

template<typename CategorizedEntryT> AFR_DetFamID
DetectorCatalogue<CategorizedEntryT>::_V_get_category_key(AFR_DetSignature k ) const {
    AFR_UniqueDetectorID id(k);
    return detector_family_num( id.byNumber.major );
}

template<typename CategorizedEntryT> const char *
DetectorCatalogue<CategorizedEntryT>::_V_get_category_name( AFR_DetFamID fk ) const {
    return detector_family_name( fk );
}

}  // namespace mixins
}  // namespace sV

# endif  // H_STROMA_V_TFILE_TDIRECTORY_DISPATCHER_MIXIN_H


/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
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

# ifndef H_STROMA_V_SVBP_READER_BUCKETS_DATA_SOURCE_H
# define H_STROMA_V_SVBP_READER_BUCKETS_DATA_SOURCE_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/dsources/svbs/bucketStreamReader.tcc"
# include "analysis/dsources/svbs/aux.hpp"
# include "analysis/dsources/svbs/recvSyncServer.hpp"
# include "utils.h"

# include <goo_dict/parameters/path_parameter.hpp>
# include <fstream>

namespace sV {

/**@class Buckets
 * @brief Compressed buckets streaming handle available as sV events source.
 *
 * Offers a wrapper built around multiple STL streams providing random access
 * to serialized deflated buckets and their supplementary info indexed
 * by SHA256 hash.
 *
 * At this level of abstraction we still have no implications about event ID.
 * */
class Buckets : public sV::buckets::BucketStreamReader<
                                aux::SHA256BucketHash,
                                events::CommonBucketDescriptor>,
                public AbstractApplication::ASCII_Entry {
public:
    typedef buckets::BucketStreamReader< aux::SHA256BucketHash,
                                         events::CommonBucketDescriptor> Parent;
private:
    /// List of sources paths.
    std::vector<goo::filesystem::Path> _paths;
    /// Indicates whether the last invokation of _V_next_event() performed
    /// successfully.
    bool _lastEvReadingWasGood;
    /// Progressbar parameters.
    PBarParameters * _pbParameters;  ///< set to nullptr when unused
    size_t _maxEventsNumber, ///< Maximum events number to read. 0 is for all available.
           _eventsRead;  ///< Counter of read events.
    /// Own container for aggregated collector instances performing supp. info
    /// deserialization by ancestor class (see SuppInfoBucketReader).
    mutable std::unordered_map<std::string, sV::buckets::iAbstractInfoCollector *> _collectors;
    /// Own container for aggregated decompressor instances involved into
    /// buckets decompression procedures used by ancestor class
    /// (see CompressedBucketReader).
    mutable Buckets::Decompressors _decompressors;
    /// Iterator referring to current source in a list.
    std::vector<goo::filesystem::Path>::const_iterator _sourcesIt;

    /// Own instance referring to reading file source.
    std::ifstream _file;

    /// Recieving buffer size. Upon recieving incoming connection, this initial
    /// size of buffer will be used. Larger data will cause this buffer to be
    /// reallocated (with this step). It doesn't really matter what size to
    /// select, but may have minor impact on performance.
    size_t _recvBufferSize;
    /// Recieving buffer container. Will be wrapped into stringstream to
    /// provide compatibility with stream-reading routines of ancestors
    /// remaining their logic intact.
    std::string _recvBuffer;
    /// Interim stream where recieved data will be kept.
    std::stringstream _recvStream;
    /// Referring to the synchroneous server that may be used for recieving
    /// messages.
    buckets::RecievingServer _reciever;
protected:
    /// Returns C++ RTTI type hash for given supp info data type using the sV's
    /// system VCtr dict.
    virtual std::type_index _V_get_collector_type_hash( const std::string & ) const override;
    /// Allocates new cache entry with set name and collector ptr fields.
    virtual MetaInfoCache _V_new_cache_entry( const std::string & ) const override;
    /// Returns true, while there are available sources remained in the list.
    virtual bool _V_is_good() override;
    /// Sets up reading from first source and reads the first available event.
    virtual Event * _V_initialize_reading() override;
    /// Causes re-acquizition of available bucket from stream. If there is no
    /// more buckets in stream, opens next in a list.
    virtual bool _V_acquire_next_bucket( BucketReader::Event *& epr ) override;
    /// Instead of raising an exception, creates a new decompressor.
    virtual const iDecompressor * _decompressor( iDecompressor::CompressionAlgo ) const override;

    /// ... TODO: doc
    static void _recieve_incoming_bucket(net::PeerConnection *);

    void _open_source( const std::string & );
public:
    /// Common C++ ctr.
    Buckets( events::DeflatedBucket * dfltdBcktPtr,
             events::Bucket * bucketPtr,
             events::BucketInfo * bInfo,
             events::CommonBucketDescriptor * particularInfo,
             const std::list<goo::filesystem::Path> & paths,
             size_t nMaxEvents,
             int portNo,
             size_t recvBufferSize,
             bool enablePBar=false,
             uint8_t ASCII_lines=1);
    /// VCtr interfacing ctr.
    Buckets( const goo::dict::Dictionary & );
    /// Frees resources.
    ~Buckets();
};  // class Buckets

}  // namespace sV

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_SVBP_READER_BUCKETS_DATA_SOURCE_H


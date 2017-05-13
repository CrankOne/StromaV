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

# ifndef H_STROMA_V_SVBP_READER_H
# define H_STROMA_V_SVBP_READER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/pipeline.hpp"
# include "utils.h"

# include <fstream>

namespace sV {

/// Performs basic reading from events bucket.
class BucketsReader : public sV::aux::iEventSequence,
                      public sV::AbstractApplication::ASCII_Entry {
public:
    typedef sV::aux::iEventSequence Parent;
    typedef typename sV::AnalysisPipeline::Event Event;
private:
    events::Bucket * _cBucket;
protected:
    virtual bool _V_is_good() override;
    virtual void _V_next_event( Event *& ) override;
    virtual Event * _V_initialize_reading() override;
    virtual void _V_finalize_reading() override;
    virtual void _V_print_brief_summary( std::ostream & ) const override;
public:
    BucketsReader( events::Bucket * reentrantBucketPtr );

    events::Bucket & bucket();

    const events::Bucket & bucket() const;

    const events::BucketMetaInfo & meta_info() const;
};


/// Manages compressed buckets.
class CompressedBucketsStreamReader : public BucketsStreamReader {
private:
    /// Reentrant compressed bucket instance.
    events::DeflatedBucket * _cDefltdBucket;
    /// Internal cache of used decompressors.
    std::unordered_map<iDecompressor::CompressionAlgo, iDecompressor *> _decompressors;
};  // class CompressedBucketsStreamReader


/// Steeres buckets file reading.
class BucketsFileReader : public CompressedBucketsStreamReader {
private:
    std::vector<goo::filesystem::Path> _filenames;
    std::ifstream _file;
    size_t _nEventsMax;
    PBarParameters * _pbParameters;  ///< set to nullptr when unused
public:
    BucketsFileReader( const std::list<goo::filesystem::Path> & filenames,
                       size_t maxEvents=0,
                       bool enableProgressbar=false );
    virtual ~BucketsFileReader();
};  // class BucketsFileReader

}  // namespace sV

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_SVBP_READER_H


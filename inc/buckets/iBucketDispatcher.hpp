/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * Author: Bogdan Vasilishin <togetherwithra@gmail.com>
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
# ifndef H_STROMA_V_IBUCKET_DISPATCHER_H
# define H_STROMA_V_IBUCKET_DISPATCHER_H

# include "config.h"

# ifdef RPC_PROTOCOLS

# include "uevent.hpp"
# include <boost/program_options.hpp>

namespace sV {

namespace po = boost::program_options;

class iBucketDispatcher {

    public:
        typedef sV::events::Bucket Bucket;

        iBucketDispatcher( size_t nMaxKB, size_t nMaxEvents );
        virtual ~iBucketDispatcher();

        virtual void push_event(const events::Event & reentrantEvent);

        size_t n_max_KB() const {return _nMaxKB;};
        size_t n_max_Events() const {return _nMaxEvents;};

        size_t n_KB() const {
            return (size_t)(1024*_currentBucket.ByteSize());
        };
        size_t n_Events() const {
            return (size_t)(_currentBucket.events_size());
        };

        virtual bool is_bucket_full();
        size_t drop_bucket();
        virtual void clear_bucket();
        static po::options_description _dispatcher_options();
    protected:
        virtual size_t _V_drop_bucket() = 0;

        events::Bucket _currentBucket;
    private:
        size_t _nMaxKB;
        size_t _nMaxEvents;

};  // class iBucketDispatcher

}  //  namespace sV

# endif  //  RPC_PROTOCOLS
# endif  //  H_STROMA_V_IBUCKET_DISPATCHER_H


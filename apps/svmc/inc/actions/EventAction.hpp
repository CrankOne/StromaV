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
# ifndef H_SVMC_EVENT_ACTION_H
# define H_SVMC_EVENT_ACTION_H

# include "afNA64_config.h"

# ifdef StromaV_RPC_PROTOCOLS
# include <G4UserEventAction.hh>
# include "buckets/iBucketDispatcher.hpp"
# include <ostream>

namespace svmc {

class EventAction : public G4UserEventAction {
    public:
        EventAction( sV::iBucketDispatcher * bucketDispatcher);
        virtual ~EventAction();

        virtual void BeginOfEventAction(const G4Event* ) final;
        virtual void EndOfEventAction(const G4Event* ) final;
    protected:
        sV::iBucketDispatcher * _bucketDispatcher;
        // std::ostream & _streamRef;
        // std::fstream & _fileRef;

};  // class EventAction

}  //  namespace svmc
# endif  // StromaV_RPC_PROTOCOLS
# endif  //  H_SVMC_EVENT_ACTION_H


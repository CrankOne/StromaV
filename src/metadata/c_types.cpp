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

# include "metadata/dictionary.tcc"

# include <climits>

static std::unordered_set<sV_MetadataTypeIndex> * _static_mdTypeIds = nullptr;

sV_MetadataTypeIndex
sV_generate_metadata_type_id( void * _this ) {
    typedef std::remove_pointer<decltype(_static_mdTypeIds)>::type
            IDsContainer;
    if( !_static_mdTypeIds ) {
        _static_mdTypeIds = new IDsContainer();
    }
    IDsContainer & ids = *_static_mdTypeIds;
    sV_MetadataTypeIndex idCandidate;
    uint16_t nSense = USHRT_MAX;
    do {
        idCandidate = double(USHRT_MAX)*(double(rand())/RAND_MAX);
        //XXX
        //std::cout << "New type ID generated:" << idCandidate << std::endl;
        nSense--;
    } while( ids.end() != ids.find(idCandidate) && nSense );
    if( !nSense ) {
        emraise( overflow, "Can not generate unique metadata type ID." );
    }
    return idCandidate;
}


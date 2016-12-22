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

/**@file detector_ids.h
 * @brief The file providing detector identification table and aux routines.
 * In StromaV this file just declare a set of traits designed for further
 * definition by third-party code.
 *
 * Here the union AFR_UniqueDetectorID is introduced in order to perform
 * transparent indexing of detector subsystem.
 *
 * These structs and routines are made to be used from within C code and often
 * performs merely aliasing of detectors table C++ class.
 *
 * TODO: description of terms:
 *  - detector family and detector features encoded inside family num
 *  - detector major/minor numbers
 */

# ifndef H_STROMA_V_INTERNAL_DETECTOR_IDENTIFIER_H
# define H_STROMA_V_INTERNAL_DETECTOR_IDENTIFIER_H

# include "goo_types.h"

# include <stdio.h>

/**\brief Detector family identifier hashable type. */
typedef UShort  AFR_DetFamID;
/**\brief Detector major section identifier type. Must have enough length to
 * contain all the generic detectors numbers. */
typedef UShort  AFR_DetMjNo;
/**\brief Detector minor section identifier type. Indexes particular detector,
 * instance or cell inside the detector class. */
typedef UShort  AFR_DetMnNo;
/**\brief Detector entire number containing hashable type. Have to provide
 * enough space to contain concatenated both the minor and the major numbers. */
typedef UInt    AFR_DetSignature;

/**\union AFR_UniqueDetectorID
 * \brief Unites unque detector identifier in terms of StromaV library.
 *
 * Combines two-byte-long uniq number and two-byte number
 * representation as more descriptive field. The wholenum field
 * can be used as key value in associative containers like maps/sets/etc.
 * */
union AFR_UniqueDetectorID {
    struct {
        /**\brief Major number defines a detector digest in terms of custom
         * code. It is defined as unique ordering number
         * written in first few bits.
         */
        AFR_DetMjNo major;
        /**\brief The minor number is used to encode x/y/z ordering num
         * coordinates for detectors which support it. There are no dedicated
         * fields. */
        AFR_DetMnNo minor;
    } byNumber;
    /**\brief Number congregating full detector identification. */
    AFR_DetSignature wholenum;
    # ifdef __cplusplus
    AFR_UniqueDetectorID( AFR_DetSignature num ) : wholenum(num) {}
    # endif
};

# ifdef __cplusplus
extern "C" {
# endif

/* TODO ... */

# ifdef __cplusplus
}  /* extern "C" */
# endif

# endif  /* H_STROMA_V_INTERNAL_DETECTOR_IDENTIFIER_H */


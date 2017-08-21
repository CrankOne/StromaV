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
# include "sV_config.h"

# include <stdio.h>

# ifdef SWIG
# define sV_FUNCTION_WEAK
# else
# define sV_FUNCTION_WEAK __attribute__(( weak ))
# endif
/**\brief Detector family identifier hashable type. */
typedef unsigned short  AFR_DetFamID;
/**\brief Detector major section identifier type. Must have enough length to
 * contain all the generic detectors numbers. */
typedef unsigned short  AFR_DetMjNo;
/**\brief Detector minor section identifier type. Indexes particular detector,
 * instance or cell inside the detector class. */
typedef unsigned short  AFR_DetMnNo;
/**\brief Detector instance index for major identifier. Usually a plain number,
 * but it is possible to use multivariate encoded index.*/
typedef unsigned short AFR_DetMjIndex;
/**\brief Detector entire number containing hashable type. Have to provide
 * enough space to contain concatenated both the minor and the major numbers. */
typedef unsigned int   AFR_DetSignature;

/**\union AFR_UniqueDetectorID
 * \brief Unites unque detector identifier in terms of StromaV library.
 *
 * Combines two-byte-long uniq number and two-byte number
 * representation as more descriptive field. The wholenum field
 * can be used as key value in associative containers like maps/sets/etc.
 * */
struct AFR_DetNumsPair {
    /**\brief Major number defines a detector digest in terms of custom
     * code. It is defined as unique ordering number
     * written in first few bits.
     */
    AFR_DetMjNo major;
    /**\brief The minor number is used to encode x/y/z ordering num
     * coordinates for detectors which support it. There are no dedicated
     * fields. */
    AFR_DetMnNo minor;
};

union AFR_UniqueDetectorID {
    struct AFR_DetNumsPair byNumber;
    /**\brief Number congregating full detector identification. */
    AFR_DetSignature wholenum;
    # ifdef __cplusplus
    AFR_UniqueDetectorID( AFR_DetSignature num ) : wholenum(num) {}
    # endif
};

# ifdef __cplusplus
extern "C" {
# endif

enum DSuL_BinCompOperatorCode {
    DSuL_Operators_none = 0,
    /* --- */
    DSuL_Operators_lt   = 1,
    DSuL_Operators_lte  = 2,
    DSuL_Operators_gt   = 3,
    DSuL_Operators_gte  = 4,
    /* --- */
    DSuL_Operators_and      = 11,
    DSuL_Operators_andNot   = 12,
};

struct sV_DSuL_MVarIndex {
    uint16_t nDim;
    union {
        uint16_t x[7];  /* <- TODO: make configurable? */
        char * strID;
    } components;
};

struct sV_DSuL_MVarIdxRange {
    struct sV_DSuL_MVarIndex first;
    union {
        struct sV_DSuL_MVarIndex border;
        enum DSuL_BinCompOperatorCode binop;
    } second;
};

struct sV_DSuL_MVarIdxRangeLst {
    struct sV_DSuL_MVarIdxRange self;
    enum DSuL_BinCompOperatorCode binop;
    struct sV_DSuL_MVarIdxRangeLst * next;
};

struct sV_DSuL_MjSelector {
    /* Setting this to major number means exact match. Otherwise has to be
     * set to family number. Apply family bitmask to get whether this is a
     * exact match or a family specifier: */
    AFR_DetMjNo majorNumber;
    struct sV_DSuL_MVarIdxRangeLst * range;
};

struct sV_DSuL_Selector {
    struct sV_DSuL_MjSelector mjSelector;
    struct sV_DSuL_MVarIdxRangeLst * range;
};

struct sV_DSuL_Expression {
    struct sV_DSuL_Selector left;
    enum DSuL_BinCompOperatorCode binop;
    struct sV_DSuL_Expression * next;
};

# if 0
AFR_DetMjNo __AFR_detector_major_by_name_dft( const char * mjName );
AFR_DetFamID __AFR_family_id_by_name_dft( const char * famName );
AFR_DetMjNo __AFR_compose_detector_major_dft(
                            AFR_DetFamID fmID,
                            const struct sV_DSuL_MVarIndex * mvIdx );
void __AFR_decode_minor_to_indexes_dft( AFR_DetMnNo minorNo,
                                        struct sV_DSuL_MVarIndex * mvIdx );
# endif

/** Returns detector major number referenced with given string expression. */
AFR_DetMjNo AFR_detector_major_by_name( const char * )
    sV_FUNCTION_WEAK;

/** Returns family identifier referenced with given string expression. */
AFR_DetFamID AFR_family_id_by_name( const char * )
    sV_FUNCTION_WEAK;

/** Returns encoded major detector descriptor referenced with given family
 * number and multivariate indexes instance. Supports second parameter to be
 * NULL --- then returns major with only family number encoded. */
AFR_DetMjNo AFR_compose_detector_major( AFR_DetFamID, const struct sV_DSuL_MVarIndex * )
    sV_FUNCTION_WEAK;

/** Writes minor multivariate indexes decoded from given detector descriptor.
 * Note, that full signature is required since index encoding may vary
 * depending of particular detector family. */
void AFR_decode_minor_to_indexes( AFR_DetSignature, struct sV_DSuL_MVarIndex * )
    sV_FUNCTION_WEAK;

char * snprintf_detector_name( char * buffer, size_t bufferSize, union AFR_UniqueDetectorID )
    sV_FUNCTION_WEAK;

# ifdef DSuL
/** Produces new selector instance from given string performing lexical
 * analysis and parsing using YACC/LEXX grammar.
 *
 * Implemented inside */
struct sV_DSuL_Expression * DSuL_compile_selector_expression( const char * );
# endif

# ifdef __cplusplus
}  /* extern "C" */
# endif

# endif  /* H_STROMA_V_INTERNAL_DETECTOR_IDENTIFIER_H */


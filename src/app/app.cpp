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

# include <goo_versioning.h>
# include <goo_exception.hpp>

# include "app/abstract.hpp"
# include "utils.hpp"

# ifdef RPC_PROTOCOLS
# include "uevent.hpp"
# endif

# include "utils.hpp"

//
// C wrappers
////////////

# ifdef __cplusplus
extern "C" {
# endif

void
sV_C_message( const int8_t level, const char * fmt, ... ) {
    char dest[GOO_EMERGENCY_BUFLEN];
    va_list argptr;
    va_start(argptr, fmt);
        vsnprintf(dest, GOO_EMERGENCY_BUFLEN, fmt, argptr);
    va_end(argptr);
    if( sV::AbstractApplication::exists() && goo::app<sV::AbstractApplication>().ls_is_set() ) {
        goo::app<sV::AbstractApplication>().message(level, dest);
    } else {
        if( -2 == level ) {
            eprintf( "StromaV : %s", dest );
        } else if( -1 == level ) {
            wprintf( "StromaV : %s", dest );
        } else {
            char  prfxBf[512];
            snprintf(prfxBf, 256, ESC_BLDCYAN "[%d%7s]" ESC_CLRCLEAR " %s", (int) level, hctime(), dest );
            fputs(prfxBf, stdout);
        }
    }
}

uint8_t
sV_get_verbosity() {
    if( sV::AbstractApplication::exists() ) {
        return goo::app<sV::AbstractApplication>().verbosity();
    } else {
        return UCHAR_MAX;
    }
}

uint8_t
sV_is_app_initialized() {
    if( sV::AbstractApplication::exists() ) {
        return 1;
    } else {
        return 0;
    }
}

# ifdef __cplusplus
}  // extern "C"
# endif


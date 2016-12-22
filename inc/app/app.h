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

# ifndef H_STROMA_V_APPLICATION_C_WRAPPERS_H
# define H_STROMA_V_APPLICATION_C_WRAPPERS_H

# include "../config.h"

# ifndef __cplusplus
#   include <stdint.h>
# else
#   include <cstdint>
# endif

/*
 * C interface extras
 */

# ifdef __cplusplus
extern "C" {
# endif

/**@brief Variadic arguments function for logging.
 *
 * Internally forwards all arguments except the `level` to vsnprintf() STD
 * C-function. Using of `level` allows to prevent loquacious output.
 *
 * Note, that if there is no the sV::AbstractApplication instance, all
 * output will be done to `stdout`/`stderr`.
 */
void sV_C_message( const int8_t level, const char * fmt, ... );

/**@brief Returns current verbosity level.
 *
 * If sV::AbstractApplication instance was constructed for the moment of
 * invokation, returns current verbosity level. Otherwise, returns `UCHAR_MAX`.
 * */
uint8_t sV_get_verbosity();

/**@brief Returns 1 if sV::AbstractApplication instance was ctrd for the
 * moment. */
uint8_t sV_is_app_initialized();

# define sV_log1(...) sV_C_message(  1, __VA_ARGS__ )
# define sV_log2(...) sV_C_message(  2, __VA_ARGS__ )
# define sV_log3(...) sV_C_message(  3, __VA_ARGS__ )
# define sV_logw(...) sV_C_message( -1, __VA_ARGS__ )
# define sV_loge(...) sV_C_message( -2, __VA_ARGS__ )

# ifdef __cplusplus
}
# endif

# endif  /* H_STROMA_V_APPLICATION_C_WRAPPERS_H */

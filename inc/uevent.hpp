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

# ifndef H_STROMA_V_UNIV_EVENT_STRUCTS_H
# define H_STROMA_V_UNIV_EVENT_STRUCTS_H

# include "config.h"

# ifndef RPC_PROTOCOLS
# warning This header requires RPC_PROTOCOLS option to be enabled \
in StromaV lib build. Possibly, some data analysis sources have not proper \
ifdef/endif config options exclusion. Expecting troubles ahead. Please, \
checkout newer revisions, or make a bugreport.
# endif

// A trick to disable shadowing warnings in aut-generated GPB
// headers.
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wshadow"
# include "event.pb.h"
# pragma GCC diagnostic pop

# include "detector_ids.h"

# endif  // H_STROMA_V_UNIV_EVENT_STRUCTS_H


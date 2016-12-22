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

/**
 * This example demonstrates typical usage of the task driven app
 * task callback implementation.
 * */

# include <cstdlib>
# include "app/task_driven.hpp"
// # include ...

namespace examples {

static int
_static__my_example_task( const p348::po::variables_map & parameters ) {
    // ...
    // Here is what we're going to do.
    // ...
    return EXIT_SUCCESS;
}

static int
_static__my_example_task_cleaner() {
    // ...
    // Here is what we're going to do after all tasks was finished.
    // ...
    return EXIT_SUCCESS;
}

_BASE_GOO_TD_APP_PUSH_TASK(
    ::p348::TaskDrivenApplication,
    "my_task_name",
    "Here be the description of my_task.",
    _static__my_example_task,
    _static__my_example_task_cleaner )

}  // namespace examples




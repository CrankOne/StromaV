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

# ifndef H_STROMA_V_TASK_DRIVEN_APPLICATION_H
# define H_STROMA_V_TASK_DRIVEN_APPLICATION_H

# include "app/abstract.hpp"

# include <unordered_map>
# include <unordered_set>

namespace sV {

/**@class TaskDrivenApplication
 * @brief Task-driven template application design interface.
 *
 * Task-driven architecture is one another way to provide modularity
 * of self-prototyping design. Particular utility code, in the case,
 * will consist of a set of tasks --- e.g., routines which will be invoked
 * in order by specifying them from userspace in some way (usually, from
 * command line). Basically the run() invokation without this tasks
 * specified does nothing.
 *
 * Data anlysis application is a more specific variation of this idea. Despite
 * this both types have common features, there is no way to provide inheritance
 * easily.
 *
 * @ingroup app analysis
 * */
class TaskDrivenApplication : virtual public AbstractApplication {
public:
    typedef AbstractApplication Parent;
    typedef Parent::Config Config;
    /// Task callback signature.
    typedef int (*TaskCallback)( const Config & );
    /// Task cleaner signature --- things that should be done after task is finished.
    typedef int (*TaskCleaner)();
protected:
    static std::unordered_map< std::string,
                        std::pair<std::string, TaskCallback> > _tasks;
    static std::unordered_set<TaskCleaner> _tskCleaners;
    /// Runs task in order; runs cleaners then.
    virtual int _V_run() final;
    /// Calls cleaners.
    virtual int _V_quench( int runRC );
    virtual std::vector<goo::dict::Dictionary> _V_get_options() const override;
    virtual void _V_configure_concrete_app() override;
public:
    TaskDrivenApplication( Config * );

    static void add_task_callback( const std::string & name,
                            TaskCallback cllb,
                            const std::string & description );
    static void add_task_cleaner( TaskCleaner clrInst );
    static void list_callbacks( std::ostream & os );
};

/**@def _BASE_GOO_TD_APP_PUSH_TASK
 * @brief Macro that implicitly adds the task to \ref TaskDrivenApplication
 * registry.
 *
 * One can provide this macro after task callback definition. Note, that
 * particular application is encouraged to wrap this macro into its own
 * aliases.
 *
 * @example examples/task_driven_task.cpp
 * @ingroup app
 */
# define _BASE_GOO_TD_APP_PUSH_TASK( appname, name, descr, callback, cleanerCallback )  \
static void __task_ ## callback ## _pusher() __attribute__((constructor (101)));        \
static void __task_ ## callback ## _pusher() {                                          \
    appname ::add_task_callback(name, callback, descr);                                 \
    appname ::add_task_cleaner(cleanerCallback);                                        \
}

}  // namespace sV

# endif  // H_STROMA_V_TASK_DRIVEN_APPLICATION_H


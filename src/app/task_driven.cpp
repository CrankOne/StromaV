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

# include "app/task_driven.hpp"

namespace sV {

std::unordered_map< std::string,
                    std::pair<std::string, TaskDrivenApplication::TaskCallback> >
TaskDrivenApplication::_tasks;

std::unordered_set<TaskDrivenApplication::TaskCleaner>
TaskDrivenApplication::_tskCleaners;

std::vector<po::options_description>
TaskDrivenApplication::_V_get_options() const {
    std::vector<po::options_description> res = Parent::_V_get_options();
    po::options_description taskDrivenAppOptions;
    { taskDrivenAppOptions.add_options()
        ("task,t",
            po::value< std::vector<std::string> >(),
            "Sets task(s).")
        ("list-tasks",
            "Print list of available tasks and exit.")
        ;
    } res.push_back(taskDrivenAppOptions);
    return res;
}

void
TaskDrivenApplication::_V_configure_concrete_app() {
    if(co().count("build-info")) {
        list_callbacks( std::cout );
        _immediateExit = true;
    }
}

int
TaskDrivenApplication::_V_run() {
    std::vector<std::string> tasksList;
    if( !co().count("task") ) {
        std::cerr << "Error -- no task specified. Available ones:" << std::endl;
        list_callbacks( std::cerr );
        return EXIT_FAILURE;
    }
    tasksList = this->cfg_option< std::vector<std::string> >("task");
    int res = 0;
    for( auto taskIt  = tasksList.begin();
     tasksList.end() != taskIt; ++taskIt ) {
        typename DECLTYPE(_tasks)::iterator targetTaskIt = _tasks.find( *taskIt );
        if( _tasks.end() == targetTaskIt ) {
            emraise( noSuchKey, "Have no task named \"%s\"", taskIt->c_str() );
        }
        res |= targetTaskIt->second.second( co() );
    }
    res |= _V_quench( res );
    return res;
}

int
TaskDrivenApplication::_V_quench( int runRC ){
    for( auto it = _tskCleaners.begin();
                   _tskCleaners.end() != it; ++it ) {
        if( *it ) {
            runRC |= (*it)();
        }
    }
    return runRC;
}

TaskDrivenApplication::TaskDrivenApplication( Config * cfg ) : Parent( cfg ) {}

void
TaskDrivenApplication::add_task_callback( const std::string & name,
                        TaskCallback cllb,
                        const std::string & description ){
    _tasks[name] = std::pair<const std::string, TaskCallback>( description, cllb );
}

void
TaskDrivenApplication::add_task_cleaner( TaskCleaner clrInst ) {
    _tskCleaners.insert( clrInst );
}

void
TaskDrivenApplication::list_callbacks( std::ostream & os ) {
    for( auto it = _tasks.begin(); it != _tasks.end(); ++it ) {
        os << " " << it->first << std::endl
           << "    " << it->second.first << std::endl;
    }
    if( _tasks.empty() ) {
        os << "<no tasks available>" << std::endl;
    }
}

}  // namespace sV


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

# ifndef H_STROMA_V_APP_MONITORING_H
# define H_STROMA_V_APP_MONITORING_H

# include <goo/goo_exception.hpp>

# include <mutex>
# include <condition_variable>
# include <thread>
# include <set>

namespace sV {
namespace aux {

/**@brief A collateral (side) thread representation operating with heavy
 *        resource.
 * @class iCollateralJob
 *
 * This template helper class is a dedicated wrapper for thread operating with
 * some expensive but non-crucial resource (like printing terminal) which
 * should be updated as it is available.
 *
 * Its usual case is the monitoring routine which has to print diagnostic and
 * statistical information, but has not to affect performance of major routine.
 *
 * It holds two controlling mutexes to synchronize lazy task processing among
 * the multiple threads. The first one (called _m) is merely an auxilliary
 * mutex for notifying printing thread when processing is desirable. The second
 * (_pM) is a mutex for task parameters. Consuming entities have to refresh
 * task parameters upon the second mutex is available AND the major co-routine
 * is pending.
 *
 * To implement such a thing:
 *
 *  - One should implement the operating code at overriden _V_sr_use() method.
 *  - The consuming classes have to inherit the nested Consumer class and then
 *    either:
 *      1) 
 *      2) Try to acquire task parameters mutex with Conumer::try_acquire()
 *      method. If Conumer::try_acquire() returned false, consumers must not
 *      to access parameters. Upon the modification of parameters is completed,
 *      the consumer must realease the lock with Conumer::release_locked()
 *      method. This approach is not recommended, but allows more sophisticated
 *      design.
 *      
 *
 * */
template< typename ParametersT
        , typename MutexT=std::mutex
        , typename CondVarT=std::condition_variable
        , typename ThreadT=std::thread
        , typename ScopedLockT=std::lock_guard<MutexT>
        , typename MoveableLockT=std::unique_lock<MutexT> >
class iCollateralJob {
public:
    typedef MutexT      Mutex;
    typedef CondVarT    CondVar;
    typedef ThreadT     Thread;
    typedef ScopedLockT ScopedLock;
    typedef MoveableLockT MoveableLock;
    typedef ParametersT Parameters;

    typedef iCollateralJob< Parameters
                          , Mutex
                          , CondVar
                          , Thread
                          , ScopedLock
                          , MoveableLock > Self;

    class Consumer {
    private:
        Self * _rPtr;
    protected:
        Consumer() : _rPtr( nullptr ) {}
        Consumer( Self & jRef ) : _rPtr(&jRef) {}
        virtual ~Consumer() {}
        /// Must be called by destructor of a resource instance.
        void _unleash() { _rPtr = nullptr; }
        /// Returns ptr to owning collateral process (can be null for unbound).
        Self * owner() { return _rPtr; }
        /// Returns ptr to owning collateral process (can be null for unbound)
        /// (const ver).
        const Self * owner() const { return _rPtr; }
    public:
        /// Returns true, if consuming resource is available.
        bool try_acquire() { return owner() ? 
                             ( owner()->is_ready() ? owner()->_pM.try_lock() : false )
                             : false; }

        /// Has to be invoked ONLY when try_acquire() succeeded.
        void release_locked() {
            owner()->_pM.unlock();
        }

        /// Should be invoked upon successfull update.
        void set_updated() {
            Consumer::owner()->_updated = true;
        }

        /// Moving sematics for scoped lock.
        friend class iCollateralJob< Parameters
                                   , Mutex
                                   , CondVar
                                   , Thread
                                   , ScopedLock
                                   , MoveableLock >;
    };

    class iConsumer : public Consumer {
    protected:
        /// Shall return true, if parameters were really modified.
        virtual bool _V_modify_collateral_job_parameters( Parameters & ) = 0;
        iConsumer( Self & jRef ) : Consumer( jRef ) {}
    public:
        virtual void modify_collateral_job_parameters() {
            if( !Consumer::_rPtr ) {
                emraise( badState, "Consumer job consumer %p is not bound to "
                    "any owner.", this );
            }
            if( Consumer::try_acquire() ) {
                Consumer::owner()->_updated |=
                        _V_modify_collateral_job_parameters(
                                            Consumer::owner()->parameters() );
                Consumer::release_locked();
            }
        }
    };
private:
    Parameters & _psRef;

    bool _isReady   ///< Spurious trigger protector
       , _doQuit    ///< Quit indicator
       , _updated   ///< Prevents duplicated refreshing
       ;
    Mutex _m;
    CondVar _cv;
    Thread _t;

    /// Tracks existance of any created consumer for job instance.
    std::set<Consumer *> _consumers;
protected:
    /// Controls task parameters access.
    Mutex _pM;

    /// Pending method, whaiting for particular condition.
    virtual bool _wait() {
        MoveableLock lock(_m);  // TODO: make LockT here!
        while( _isReady && !_doQuit ) {
            _cv.wait( lock );
        }
        if( _doQuit ) {
            return false;
        }
        return true;
    }
    /// Abstract method performing actual work.
    virtual void _V_sr_use( Parameters & ) = 0;
    /// Will sequentially perform task.
    virtual void _sr_run() {
        while( _wait() ) {
            MoveableLock jobLock(_m);
            _V_sr_use( _psRef );
            _isReady = true;
            _updated = false;
        }
    }

    Parameters & parameters() { return _psRef; }
public:
    iCollateralJob( Parameters & psRef ) : _psRef(psRef)
                                         , _isReady( true )
                                         , _doQuit( false )
                                         , _updated( true )
                                         , _t( std::bind( &Self::_sr_run, this ) )
                                         {;}

    virtual ~iCollateralJob() {
        for( auto c : _consumers ) {
            c->_unleash();
        }
        {
            MoveableLock l(_m);
            _doQuit = true;
            _cv.notify_all();
        }
        _t.join();
    }

    void notify() {
        if( _updated && is_ready() && _m.try_lock() ) {
            _isReady = false;
            _m.unlock();
            _cv.notify_all();
        }
    }
    virtual bool is_ready() const { return _isReady; }
};  // class iCollateralJob

}  // namespace aux
}  // namespace sV

# endif  // H_STROMA_V_APP_MONITORING_H


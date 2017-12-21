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

# include "app/collateral_job.tcc"
# include <unistd.h>
# include <vector>
# include <sstream>

# include <iostream>  // XXX
# include <algorithm>

/* Such test has to ensure that:
 *  - Job does not perform anything while condition variable is not notified
 *  - Parameters mutex correctly controls ownership of Parameter structure
 * The busy timings are choosen randomly during the evaluation of three
 * simultaneous threads:
 *  1. The major execution thread that represents consumer #1, modifying the
 *     parameter struct.
 *  2. The collateral thread managing "expensive" resource
 *  3. The minor execution thread that represents consumer #2, modifying the
 *     parameter struct.
 */

// Controls minimal timing unit.
# define MAX_BUSYNESS_TU 250
# define MAX_ITERATIONS 1000
# define RANDOM_INTERVAL (std::rand()/double(RAND_MAX))*MAX_BUSYNESS_TU

namespace sV {
namespace test {

struct JobParameters {
    // number of times, each thread was owner of parameters structure:
    unsigned int n[3];
};


class Job : public aux::iCollateralJob<JobParameters> {
public:
    typedef typename aux::iCollateralJob<JobParameters>::Self Parent;
private:
    uint8_t _timings[3];
    std::vector<JobParameters> _history;
protected:
    virtual void _V_sr_use( JobParameters & ps ) override {
        // Pretend, that we're re-initializing resource for something.
        usleep( RANDOM_INTERVAL*_timings[0] );
        // Pretend, that we're doing something with parameter struct.
        {
            Parent::MoveableLock guard(_pM);  // Not needed?
            //printf( "%u, %u, %u\n", ps.n[0], ps.n[1], ps.n[2] );  // dbg
            _history.push_back(ps);
            usleep( RANDOM_INTERVAL*_timings[1] );
        }
        // Pretend, that we're freeing resource.
        usleep( RANDOM_INTERVAL*_timings[2] );
    }
public:
    Job( uint8_t tPrior,
         uint8_t tBusiness,
         uint8_t tAfter,
         JobParameters & jp ) : Parent(jp)
                              , _timings{tPrior, tBusiness, tAfter}
                              {}
    std::vector<JobParameters> history() const { return _history; }
};

class Consumer : public Job::iConsumer {
private:
    int _myNum;
protected:
    virtual bool _V_modify_collateral_job_parameters( JobParameters & ps ) override {
        ps.n[_myNum] += 1;
        return true;
    }
public:
    Consumer( int myNum, Job & j ) : Job::iConsumer( j ), _myNum(myNum) {}
};

void third_thread( Job & j  // owning job reference
                 , int nt   // internal thread ID
                 , uint8_t nTiming  // timing fraction for parameter modification
                 ) {
    Consumer c( nt, j );
    for( size_t n = 0; n < MAX_ITERATIONS; ++n ) {
        c.modify_collateral_job_parameters();
        usleep( RANDOM_INTERVAL*nTiming );
    }
}

std::vector<JobParameters>
run_tcase( uint8_t tPrior,
           uint8_t tBusiness,
           uint8_t tAfter,
           uint8_t tThread,
           uint8_t tMThread ) {
    JobParameters jp = {{ 0, 0 }};  // shared job parameters instance

    Job cj( tPrior
          , tBusiness
          , tAfter
          , jp );  // will maintain thread #2
    Consumer c1(0, cj);  // Will be maintained from within major thread

    std::thread t3( third_thread, std::ref( cj ), 1, tThread );
    std::thread t4( third_thread, std::ref( cj ), 2, tThread );

    for( size_t n = 0; n < MAX_ITERATIONS; ++n ) {
        c1.modify_collateral_job_parameters();
        usleep( RANDOM_INTERVAL*tMThread );
        cj.notify();
    }
    // printf( "Thread #1 loop done.\n" );  // dbg
    t3.join();
    t4.join();
    // printf( "Threads #3,#4 joined.\n" );  // dbg

    return cj.history();
}

std::string
analyze_history( const std::vector<JobParameters> & h ) {
    std::stringstream ss;
    ss << "#" << h.size() << ":";
    double mean[3] = {0, 0, 0}  // "mean delta"
         , var[3] = {0, 0, 0}   // "variance" (similar to dispersion)
         ;
    if( h.size() < 2 ) {
        ss << "trivial" << std::endl;
        return ss.str();
    }
    for( auto e : h ) {
        for( uint8_t i = 0; i < 3; ++i ) {
            mean[i] += e.n[i] - var[i];
            var[i] = e.n[i];  // var is used as "previous" here
            //std::cout << e.n[i] << " ";  // XXX
        }
        //std::cout << std::endl;  // XXX
    }
    for( uint8_t i = 0; i < 3; ++i ) {
        mean[i] /= h.size();
        var[i] = 0;
    }
    for( auto e : h ) {
        for( uint8_t i = 0; i < 3; ++i ) {
            var[i] += fabs( mean[i] - fabs(e.n[i] - var[i]) );
        }
    }
    for( uint8_t i = 0; i < 3; ++i ) {
        var[i] /= h.size();
        ss << "(" << (int) i << ")=" << mean[i] << "[+/-]" << var[i] << ", ";
    }
    return ss.str();
}

}  // namespace test
}  // namespace sV

//
// Test suite

# define BOOST_TEST_NO_MAIN
//# define BOOST_TEST_MODULE Data source with metadata
# include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Parallel_suite )

BOOST_AUTO_TEST_CASE( CollateralJobTC ) {

    //std::srand(unsigned(std::time(0)));
    //std::vector<sV::test::JobParameters> histories;

    //sV::test::for_all_timings( 5, 3 );
    std::stringstream ss;
    //std::cout
    ss
        << analyze_history( sV::test::run_tcase( 1, 3, 1, 3, 1 ) )
        << std::endl
        << analyze_history( sV::test::run_tcase( 0, 1, 0, 2, 3 ) )
        << std::endl
        << analyze_history( sV::test::run_tcase( 3, 1, 0, 1, 5 ) )
        << std::endl
        ;
    // ^^^ usual output is something like:
    //  #180:(0)=0.566667[+/-]0.563519, (1)=0.488889[+/-]0.486173, (2)=0.516667[+/-]0.513796, 
    //  #544:(0)=0.367647[+/-]0.366971, (1)=0.915441[+/-]0.913758, (2)=0.915441[+/-]0.913758, 
    //  #408:(0)=0.661765[+/-]0.660143, (1)=0.928922[+/-]0.926645, (2)=0.904412[+/-]0.902195,
    // Where the # number corresponds to entire updates, and the triplet
    // relates to each interesting thread. The numbers in triplet indicates
    // averaged increment and averaged increment deviation.

    //BOOST_TEST_MESSAGE( "[==] Collateral job." );
}

BOOST_AUTO_TEST_SUITE_END()



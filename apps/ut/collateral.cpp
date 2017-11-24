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
    uint8_t nIteration;
protected:
    virtual void _V_run( JobParameters & ps ) override {
        // Pretend, that we're re-initializing resource for something.
        usleep( RANDOM_INTERVAL );
        // Pretend, that we're doing something with parameter struct.
        {
            //Parent::MoveableLock guard(_pM);
            printf( "%u, %u, %u\n", ps.n[0], ps.n[1], ps.n[2] );
            usleep( RANDOM_INTERVAL );
        }
        // Pretend, that we're freeing resource.
        usleep( RANDOM_INTERVAL );
    }
public:
    Job( JobParameters & jp ) : Parent(jp) {}
};

class Consumer : public Job::iConsumer {
private:
    int _myNum;
protected:
    virtual void _V_modify_collateral_job_parameters( JobParameters & ps ) override {
        ps.n[_myNum] += 1;
    }
public:
    Consumer( int myNum, Job & j ) : Job::iConsumer( j ), _myNum(myNum) {}
};

void third_thread( Job & j, int nt ) {
    Consumer c( nt, j );
    for( size_t n = 0; n < MAX_ITERATIONS; ++n ) {
        c.modify_collateral_job_parameters();
        //printf( "#3 iteration: %d.\n", (int) n );
        usleep( 6*RANDOM_INTERVAL );
    }
    printf( "Thread #%d done.\n", nt+2 );  // XXX
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

    std::srand(unsigned(std::time(0)));

    sV::test::JobParameters jp = {{ 0, 0 }};  // shared job parameters instance

    sV::test::Job cj( jp );  // will maintain thread #2
    sV::test::Consumer c1(0, cj);  // Will be maintained from within major thread

    std::thread t3( sV::test::third_thread, std::ref( cj ), 1 );
    std::thread t4( sV::test::third_thread, std::ref( cj ), 2 );

    for( size_t n = 0; n < MAX_ITERATIONS; ++n ) {
        c1.modify_collateral_job_parameters();
        //printf( "#1 iteration: %d.\n", (int) n );
        usleep( RANDOM_INTERVAL );
        cj.notify();
    }
    printf( "Thread #1 loop done.\n" );  // XXX
    t3.join();
    t4.join();
    printf( "Threads #3,#4 joined.\n" );  // XXX

    BOOST_TEST_MESSAGE( "[==] Collateral job." );
}

BOOST_AUTO_TEST_SUITE_END()



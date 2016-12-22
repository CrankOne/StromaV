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

# include <stdlib.h>
# include <stdint.h>
# include <unistd.h>
# include <string.h>
# include <stdio.h>

# include "utils.h"

void
redraw_pbar( size_t progress, struct PBarParameters * p ) {
    uint8_t i;
    double cache;

    struct timeval now;
    gettimeofday( &now, NULL );

    if( progress != p->full ){
        if( !p->lastInvokationTime.tv_sec ) {
            p->lastInvokationTime = now;
            p->startFrom = now;
        } else {
            cache = (  ((double)(now.tv_sec - p->lastInvokationTime.tv_sec))
                     + ((double)(now.tv_usec - p->lastInvokationTime.tv_usec))*1e-6
                    );
            /*printf( "%d --- %e\r", (int) now.tv_sec, cache );*/
            if( cache < p->mtrestrict*1e-3 ){
                return;
            }
            p->lastInvokationTime = now;
        }
    }

    double progressRel = progress/((double) p->full);
    uint8_t percProgress = (uint8_t) (100*progressRel);
    uint8_t lenProgress = (uint8_t) (p->length*progressRel);
    printf( "\033[1A\r" );
    for( i = 0; i < lenProgress; ++i ) {
        printf( "█" );
    }
    if( i != p->length ) {
        printf("▒");
        for( ; i < p->length; i++ ) {
            printf( "░" );
        }
        printf( "  %d%%\n", (int) percProgress );
    } else {
        printf( "   done by %d sec.\n", (int) (now.tv_sec - p->startFrom.tv_sec) );
    }
    fflush(stdout);
}

# ifdef STANDALONE_BUILD
int
main(int argc, char * argv[]) {
    struct PBarParameters pbPars;
    int i;

    bzero( &pbPars, sizeof(struct PBarParameters) );

    pbPars.full = 1567;
    pbPars.mtrestrict = 100;
    pbPars.length = 80;

    for( i = 0; i < pbPars.full; ++i ) {
        usleep(10000);
        redraw_pbar(i, &pbPars);
    }
    redraw_pbar(1567, &pbPars);
    putchar('\n');

    return EXIT_SUCCESS;
}
# endif


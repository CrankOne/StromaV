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

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include "evGen/aprimeCrossSection.h"

static const char _static_usageMsg[] = "\
Usage:\n\
    $ %s <thetaRange> <thetaNBins> <xRange> <xNBins>\n\
Application produces 2D histogram data (map) for being used\n\
with gnuplot.\n\
";

int
main(int argc, char * argv[]) {
    uint16_t     Z = 82,
        thetaNBins,
            xNBins;
    int16_t i, j;
    double       A = 207.2,
        massAPrime = 1,
                E0 = 100,
           epsilon = 1e-5,
            xRange, xStep,
        thetaRange, thetaStep,
             sigma,
             x, theta;
    FILE * outF;
    void * wsPtr = NULL;

    if( argc != 5 ) {
        fprintf( stderr, _static_usageMsg, argv[0] );
        return EXIT_FAILURE;
    }

    init_aprime_cross_section_workspace(
                    /* unsigned short, material Z ... */ Z,
                    /* double, material A ........... */ A,
                    /* double, suggested A' mass, GeV */ massAPrime,
                    /* double, E_0, GeV ............. */ E0,
                    /* double, supposed mixing fuctor */ epsilon,
                    # define send_numsparameter_arg( type, txtName, dft, name, descr ) dft,
                        for_all_p348lib_aprimeCS_GSL_chi_comp_parameter( send_numsparameter_arg )
                    # undef send_numsparameter_arg
                    &wsPtr
    );

    thetaRange = atof( argv[1] )*M_PI;
    thetaNBins = atoi( argv[2] );
    thetaStep = 2*thetaRange/(thetaNBins-1);
    xRange = atof( argv[3] );
    xNBins = atoi( argv[4] );
    xStep = (upper_cut_x(wsPtr) - lower_cut_x(wsPtr))/(xNBins-1);


    {
        outF = fopen( "out.dat", "w" );

        fprintf( outF, "# Parameters: thetaRange -> +/- %e\n", thetaRange );
        fprintf( outF, "#             thetaNBins = %u\n", thetaNBins );
        fprintf( outF, "#             xRange -> %e...%e\n", _p348_CST_electronMass_GeV,
                                                            xRange - _p348_CST_electronMass_GeV );
        fprintf( outF, "#             xNBins = %u\n", xNBins );

        for( i = 0; i < xNBins; ++i ) {
            for( j = -thetaNBins/2.; j < thetaNBins/2.; ++j ) {
                x = i*xStep + _p348_CST_electronMass_GeV/E0;
                theta = j*thetaStep;
                sigma = aprime_cross_section( x,
                                              theta,
                                              wsPtr );
                fprintf( outF, "%e %e %e\n", x, theta, sigma );
            }
            fprintf( outF, "\n" );
        }
    }

    x = 1 - _p348_CST_electronMass_GeV/E0;
    theta = 0;
    printf( "Singular value at x = %e, theta = %e: sigma = %e < %e?\n",
            x, theta,
            aprime_cross_section( x, theta, wsPtr ),
            analytic_integral_estimation( wsPtr ) );

    free_aprime_cross_section_workspace( wsPtr );

    return EXIT_SUCCESS;
}


#!/usr/bin/env sh


#
# This file is used by main p348g4-building CMake script only when CMake-option -DGEANT4_MC_MODEL=ON.
# It dumps to a stdout a list of found Geant4 physics lists (they sometimes differs from one version to
# another).
# FMT:
#   $ get_phLists.sh <GEANT4INCDIR>
# where <GEANT4INCDIR> is usually "/usr/include/Geant4"
# It should dump to stdout something like that:
#
#   QGSP_INCLXX
#   QGSP_BERT
#   FTFP_INCLXX_HP
#   ...
#   QGSP_FTFP_BERT
#
# Please, let me fix any problems you may be faced with.
#
#                                   Crank
#

PREFIXES=""
GEANT4INCDIR=$1

find $GEANT4INCDIR -regex "^$GEANT4INCDIR/?\(QGSP\|FTFP\|LHEP\).*hh$" \
            -regextype posix-minimal-basic -type f -exec basename {} .hh \; | \
            sort | \
            uniq -u


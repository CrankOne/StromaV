#!/usr/bin/env sh

#
# This file is used by main p348g4-building CMake script only when CMake-option -DGEANT4_MC_MODEL=ON
# and -DDYNAMIC_PHYSICS=ON.
# It dumps to a stdout a list of found Geant4 physics modules (its set sometimes differs from one
# version to another).
# FMT:
#   $ get_phLists.sh <GEANT4INCDIR>
# where <GEANT4INCDIR> is usually "/usr/include/Geant4"
# It should dump to stdout something like that:
#
#   G4ChargeExchangePhysics
#   G4DecayPhysics
#   G4EmDNAPhysicsChemistry
#   ...
#   HadronPhysicsShielding
#
# Please, let me fix any problems you may be faced with.
#
#                                   Crank
#

PREFIXES=""
GEANT4INCDIR=$1

#awk '{print substr($0, 1, length($0)-1)}' file.txt

grep 'class \+.\+ : \+public \+G4VPhysicsConstructor' -R $GEANT4INCDIR | \
        awk -F":" '{print $1, $2}' | \
        awk '{print $1}' | \
        sort | \
        uniq -u | \
        while read -r line ; do basename "$line" .hh ; done | \
        grep "^G4V.\+$" -v | \
        grep "G4MiscQGSCBuilder" -v  | \
        grep "G4EmDNAPhysics_option7" -v



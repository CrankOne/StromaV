#!/bin/sh

SOURCE_FILES_ENCODED=`                          \
    find . \( -name "*.hh"   -o -name "*.cc"    \
           -o -name "*.h"    -o -name "*.c"     \
           -o -name "*.hpp"  -o -name "*.cpp"   \
           -o -name "*.ihpp" -o -name "*.icpp"  \
           -o -name "*.tcc"  -o -name "*.h.in"  \
           -o -name "*.ihpp.in" \
           \) -type f -print0 | xxd -p`

# produces hex dump
#echo -ne "$SOURCE_FILES_ENCODED" | xxd -p -r | od -c

# Prints files to be refactored:
#echo -ne "$SOURCE_FILES_ENCODED" | xxd -p -r | xargs -0 -I '{}' \
#    echo -e "{}"

#echo -ne "$SOURCE_FILES_ENCODED" | xxd -p -r | xargs -0 -I '{}' \
#    grep ::aframe "{}" -n --color

# Stages
#######

# This files one have to rename manually:
#find . -iname "*aframe*" -type f -print

# Rename ::aframe -> ::sV namespace entry
#echo -ne "$SOURCE_FILES_ENCODED" | xxd -p -r | xargs -0 -I '{}' \
#    grep ::aframe "{}" -n --color
echo -ne "$SOURCE_FILES_ENCODED" | xxd -p -r | xargs -0 -I '{}' \
    sed -i -e 's/::aframe/::sV/g' '{}'

# Rename namespace:
#echo -ne "$SOURCE_FILES_ENCODED" | xxd -p -r | xargs -0 -I '{}' \
#    grep namespace\ aframe "{}" -n --color
echo -ne "$SOURCE_FILES_ENCODED" | xxd -p -r | xargs -0 -I '{}' \
    sed -i -e 's/namespace\ aframe/namespace sV/g' '{}'

#echo -ne "$SOURCE_FILES_ENCODED" | xxd -p -r | xargs -0 -I '{}' \
#    grep '\<aframe\>' "{}" -n --color
echo -ne "$SOURCE_FILES_ENCODED" | xxd -p -r | xargs -0 -I '{}' \
    sed -i -e 's/\<aframe\>/sV/g' '{}'

# Sentinel macro:
echo -ne "$SOURCE_FILES_ENCODED" | xxd -p -r | xargs -0 -I '{}' \
    sed -i -e "s/\(H_\)AFRAME\([^[:space:]]\+_H\)/\1STROMA_V\2/g" '{}'

# variaous aframe_log?() macros:
echo -ne "$SOURCE_FILES_ENCODED" | xxd -p -r | xargs -0 -I '{}' \
    sed -i -e "s/aframe_log\([^[:space:]]\)/sV_log\1/g" '{}'


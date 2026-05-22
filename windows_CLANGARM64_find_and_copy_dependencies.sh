#!/bin/bash -ex
# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 2020 HörTech gGmbH
# Copyright © 2026 Hörzentrum Oldenburg gGmbH
#
# openMHA is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# openMHA is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License, version 3 for more details.
#
# You should have received a copy of the GNU Affero General Public License, 
# version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

# To create a Windows installer zip from msys2 compiled binaries, (ucrt64 or
# clangarm64), we need to find all msys2-provided dependencies of the binaries
# and copy them also into the destination folder.

# Invoke this script with current working directory set to the bin folder
# that needs to be extended with the dependencies.

recursion=$1
if [ -z "$recursion" ]
then
    recursion=1
fi
echo "Recursion level: $recursion"
if [ $recursion -gt 4 ]
then
    echo "Maximum recursion level reached, exiting."
    exit
fi

# Find all linked DLLs
for file in $(ls ./)
do
    llvm-objdump -p ./$file | grep "DLL Name: " | sed -e 's/.*DLL Name: //g'
done > ../tmp

sort -u ../tmp > ../tmp2
echo "Trying to copy the following dependencies:" \n
cat ../tmp2

changed=false
# Copy the files that can be found in /clangarm64/bin
for file in $(cat ../tmp2)
do
    if [ -f /clangarm64/bin/$file ]  && [ ! -f $file ];
    then
        echo "Copying $file from /clangarm64/bin to bin/ folder"
        changed=true
        cp /clangarm64/bin/$file . || true
    fi
done
rm -f tmp*
if $changed
then 
    $0 $((recursion+1))
fi

# Sanity check - see if all dependencies that we normally expect are present
for file in LIBC++ LIBOGG LIBPORTAUDIO LIBVORBISENC LIBSNDFILE LIBWINPTHREAD LIBLO LIBLSL
do
    if !(ls | grep -iq $file); then
        echo "find_and_copy_dependencies:" \n "Error: Expected $file to be present!" >&2
        exit 1
    fi
done

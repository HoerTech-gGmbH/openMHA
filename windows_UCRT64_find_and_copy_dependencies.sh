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

# Walk the dependency tree, exclude all files that are already present here and files in windows/system
for file in $(ls ./)
do
    cygpath -u $(cygcheck.exe ./$file | tr -d "[:blank:]") | grep -v "$(pwd)" | (grep -iv "/c/" || true) >> ../tmp
    if grep -iq "not find" ../tmp
    then
        echo "find_and_copy_dependencies:" \n "Error: " `grep -i "not find" ../tmp` >&2
        exit 1
    fi
done
cd ../
sort -u tmp > tmp2
echo "Need to copy the following dependencies:" \n
cat tmp2
# Sanity check - see if all dependencies that we normally expect are present
for file in LIBFLAC LIBGCC_S_SEH LIBOGG LIBPORTAUDIO LIBSNDFILE LIBSTDC++ LIBVORBIS LIBVORBISENC LIBWINPTHREAD LIBLO LIBLSL
do
    if ! grep -iq $file tmp2; then
        echo "find_and_copy_dependencies:" \n "Error: Expected $file to be in list of dependencies!" >&2
        exit 1
    fi
done
# Copy the files found by the dependency walk
for file in $(cat tmp2); do
    cp $file bin/.;
done
rm tmp*

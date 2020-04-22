#!/bin/bash -ex
# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 2020 HörTech gGmbH
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

# Copy all binaries into the destination folder
mkdir -p bin
cp ../../../../bin/* bin/.
rm bin/thismha.sh
# Walk the dependency tree, exclude all files that are already present in ./bin and files in windows/system
cd ./bin
for file in $(ls ./); do
    cygpath -u $(cygcheck.exe ./$file | tr -d "[:blank:]") | grep -v "$(pwd)" | (grep -iv "/c/" || true) >> ../tmp
    if grep -iq "not find" ../tmp; then
        echo "find_and_copy_dependencies:" \n "Error: " `grep -i "not find" ../tmp` >&2
    fi
done
cd ../
sort -u tmp > tmp2
# Sanity check - see if all dependencies that we normally expect are present
for file in $(cat expected_dependencies.txt); do
    if ! grep -iq $file tmp2; then
        echo "find_and_copy_dependencies:" \n "Expected $file to be in list of dependencies!" >&2
        exit 1
    fi
done
# Copy the files found by the dependency walk
for file in $(cat tmp2); do
    cp $file bin/.;
done
rm tmp*
# Matlab toolbox files
cp -r ../../mfiles mfiles
find ./mfiles -name ".*" -exec rm {} \;
# Examples
cp -r ../../../../examples examples
find ./examples -name ".*" -exec rm {} \;
# Reference algorithms
cp -r ../../../../reference_algorithms reference_algorithms
find ./reference_algorithms -name ".*" -exec rm {} \;
# Documentation
mkdir -p doc
cp ../../../../*pdf doc/.
cp ../../../../COPYING doc/.

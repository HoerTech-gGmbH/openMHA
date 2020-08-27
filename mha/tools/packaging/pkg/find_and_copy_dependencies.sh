#!/bin/bash -ex
# Copy all binaries into the destination folder

# remove any leftovers from last run
rm -rf bin/ lib/
mkdir -p bin lib/openmha

# copy openMHA binaries and libraries
cp -v ../../../../lib/* lib/openmha/.
cp -v ../../../../bin/* bin/.
rm -f bin/thismha.sh
chmod 755 bin/* lib/*

# adds dependencies (more libs) and corrects shared library references for the
# installation location /usr/local
function resolve_and_correct_references_of()
{
    local file="$1"
    # if this is a library, correct its own idea where it is installed
    if [[ $(dirname "$file") == "lib/openmha" ]]
    then install_name_tool -id /usr/local/"$file" "$file"
    fi

    local dependency
    otool -L "$file"
    otool -L "$file" | cut -d" " -f1 | grep -v "/usr/lib"| grep -v "/System" | \
        grep -v : | grep -v libjack | while read dependency
    do
        local installed_dependency="lib/openmha/$(basename "$dependency")"

        # resolve dependency
        if [ ! -e "$installed_dependency" ]
        then
            cp -v "$dependency" "$installed_dependency"
            chmod 755  "$installed_dependency"
            resolve_and_correct_references_of "$installed_dependency"
        fi

        # correct reference of file to dependency
        install_name_tool -change "$dependency" "/usr/local/$installed_dependency" "$file"
    done
}

# for each binary and library
for file in bin/* lib/openmha/*
do
    resolve_and_correct_references_of "$file"
done

# Wo do not want to redistribute libjack - See D191
rm -f lib/openmha/*libjack*dylib

# make sure there is no local/opt leftover reference (regression)
if grep -r local/opt lib
then exit 1
fi

#!/bin/bash -e
# Copy all binaries into the destination folder

# remove any leftovers from last run
rm -rf bin/ lib/
mkdir -p bin lib

# copy openMHA binaries and libraries
cp -v ../../../../lib/* lib/.
cp -v ../../../../bin/* bin/.
rm -f bin/thismha.sh
chmod 755 bin/* lib/*

# adds dependencies (more libs) and corrects shared library references for the
# installation location /usr/local
function resolve_and_correct_references_of()
{
    local file="$1"
    # if this is a library, correct its own idea where it is installed
    if [[ $(dirname "$file") == "lib" ]]
    then install_name_tool -id /usr/local/"$file" "$file"
    fi

    local dependency
    otool -L "$file" | cut -d" " -f1 | grep -v "/usr/lib"| grep -v "/System" | \
        grep -v : | while read dependency
    do
        local installed_dependency="lib/$(basename "$dependency")"

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
for file in bin/* lib/*
do
    resolve_and_correct_references_of "$file"
done

# We do not want to redistribute jack - see T719
rm lib/libjack*dylib

# make sure there is no local/opt leftover reference (regression)
if grep -r local/opt lib
then exit 1
fi

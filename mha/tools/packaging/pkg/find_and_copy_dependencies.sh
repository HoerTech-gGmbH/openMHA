#!/bin/bash
# Copy all binaries into the destination folder


function fix_install_names {
    file=$1;
    DYLIBS=$(otool -L ./lib/`basename $file` | grep -v "/usr/lib" | grep -v ":" | awk -F' ' '{ print $1 }' | sed '1d')
    INSTALL_NAME=$(echo @rpath/../lib/`basename $file` | sed -E 's/\.[0-9]+//g');
    install_name_tool -id $INSTALL_NAME ./lib/`basename $file`;
    for dylib in $DYLIBS; do
        LIB_NAME=$(echo @rpath/../lib/`basename $dylib`| sed -E 's/\.[0-9]+//g');
        install_name_tool -change $dylib $LIB_NAME ./lib/`basename $file`;
    done;
}
mkdir -p bin
mkdir -p lib
cp ../../../../lib/* lib/.
cp ../../../../bin/* bin/.
rm bin/thismha.sh
# Walk the dependency tree, exclude all files that are already present in ./bin and files in windows/system
fix_install_names libopenmha.dylib
for file in $(cat expected_dependencies.txt); do
    cp $file lib/.;
    fix_install_names $file;
done;

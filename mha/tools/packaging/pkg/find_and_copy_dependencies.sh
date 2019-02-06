#!/bin/bash
# Copy all binaries into the destination folder

function fix_install_names {
    path=$1
    file=$2;
    if [[ $1 == "lib" ]]; then
        INSTALL_NAME=$(echo @rpath/../lib/`basename $file` | sed -E 's/\.[0-9]+//g');
        install_name_tool -id $INSTALL_NAME ./lib/`basename $file`;
    else
        name=`basename $file`
        DYLIBS=$(otool -L ./$path/`basename $file` | grep -v "/usr/lib"| grep -v "/System" | sed "/$name/d" | Awk -F' ' '{ print $1 }')
    fi
    echo $DYLIBS
    for dylib in $DYLIBS; do
        LIB_NAME=$(echo @rpath/../lib/`basename $dylib`| sed -E 's/\.[0-9]+//g');
        install_name_tool -change $dylib $LIB_NAME ./$path/`basename $file`;
    done;
}
function find_lib {
    file=$1;
    name=`basename $file`
    DYLIBS=$(otool -L $file | grep -v "/usr/lib"| grep -v "/System" | sed "/$name/d" | awk -F' ' '{ print $1 }' | xargs -n1)
    for dylib in $DYLIBS; do
        dest=lib/$(echo `basename $dylib` | sed -E 's/\.[0-9]+//g');
        if [ ! -f $dest ]; then
            cp $dylib lib/$(echo `basename $dylib` | sed -E 's/\.[0-9]+//g');
        fi;
    done;
}

mkdir -p bin
mkdir -p lib
cp ../../../../lib/* lib/.
cp ../../../../bin/* bin/.
rm bin/thismha.sh

for file in bin/*; do
    find_lib $file;
done;

while [ "$LIBS" != "$(ls lib/)" ]; do
    LIBS=$(ls lib/)
    for file in lib/*; do
        find_lib $file;
    done;
done;

for file in lib/*dylib; do
    fix_install_names lib $file;
done;

for file in bin/*; do
    fix_install_names bin $file;
done;

for file in $(cat expected_dependencies.txt); do
    if [ ! -f lib/$file ]; then
       echo "Error: Expected dependency" $file "not found in lib/. ";
       exit 1;
    fi
done;

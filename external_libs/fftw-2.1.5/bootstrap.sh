#!/bin/sh

(cd gensrc; make)

aclocal

# sometimes automake takes a while to add all the required files :-)
automake --add-missing
automake --add-missing
automake --add-missing
automake --add-missing

autoconf

# repeat the procedure so that ``configure'' will be added to the
# distribution.
automake
autoconf

# create makefiles
sh configure

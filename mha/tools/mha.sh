# Source this script to set up the MHA build that this script is part of.

drop_from_path()
{
   # Assert that we got enough arguments
   if test $# -ne 2 ; then
      echo "drop_from_path: needs 2 arguments"
      return 1
   fi

   local p=$1
   local drop=$2

   newpath=`echo $p | sed -e "s;:${drop}:;:;g" \
                          -e "s;:${drop}\$;;g"   \
                          -e "s;^${drop}:;;g"   \
                          -e "s;^${drop}\$;;g"`
}

if [ -n "${MHASYS}" ] ; then
   old_mhasys=${MHASYS}
fi

SOURCE=${BASH_ARGV[0]}
if [ "x$SOURCE" = "x" ]; then
    SOURCE=${(%):-%N} # for zsh
fi

if [ "x${SOURCE}" = "x" ]; then
    if [ -f bin/thismha.sh ]; then
        MHASYS="$PWD"; export MHASYS
    elif [ -f ./thismha.sh ]; then
        MHASYS=$(cd ..  > /dev/null; pwd); export MHASYS
    else
        echo ERROR: must "cd where/mha/is" before calling ". bin/mha.sh" for this version of bash!
        MHASYS=; export MHASYS
        return 1
    fi
else
    # get param to "."
    thismha=$(dirname ${SOURCE})
    MHASYS=$(cd ${thismha}/.. > /dev/null;pwd); export MHASYS
fi

if [ -n "${old_mhasys}" ] ; then
   if [ -n "${PATH}" ]; then
      drop_from_path "$PATH" "${old_mhasys}/bin"
      PATH=$newpath
   fi
   if [ -n "${LD_LIBRARY_PATH}" ]; then
      drop_from_path "$LD_LIBRARY_PATH" "${old_mhasys}/lib"
      LD_LIBRARY_PATH=$newpath
   fi
   if [ -n "${DYLD_LIBRARY_PATH}" ]; then
      drop_from_path "$DYLD_LIBRARY_PATH" "${old_mhasys}/lib"
      DYLD_LIBRARY_PATH=$newpath
   fi
fi

if [ -z "${PATH}" ]; then
    PATH=${MHASYS}/bin; export PATH
else
   PATH=${MHASYS}/bin:$PATH; export PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH=${MHASYS}/lib; export LD_LIBRARY_PATH       # Linux, ELF HP-UX
else
   LD_LIBRARY_PATH=${MHASYS}/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

if [ -z "${DYLD_LIBRARY_PATH}" ]; then
   DYLD_LIBRARY_PATH=${MHASYS}/lib; export DYLD_LIBRARY_PATH   # Mac OS X
else
   DYLD_LIBRARY_PATH=${MHASYS}/lib:$DYLD_LIBRARY_PATH; export DYLD_LIBRARY_PATH
fi

unset old_mhasys
unset thismha
unset -f drop_from_path

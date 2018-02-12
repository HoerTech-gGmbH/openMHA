#!/bin/sh

# TODO: run write_matrices_cfg.m from here if it doesn't exist?

MHA_ROOT="/home/marcec/Downloads/MHA-Forschergruppe-TpA-devel-4.4.58-x86_64-linux-gcc-4.6/"
PLUGIN_PATH="plugins/"

print_help() {
    cat << EOF
$(basename $0) [-m <path>] [-p <path>] [-h] [-- <MHA options>]

This script will start MHA with the example configuration in mha_config/.  You
may pass arguments to MHA after separating them with a "--".

Options:
  -h|--help   This help text.
  -p <path>   The location of the compiled plug-ins (default: $PLUGIN_PATH).
  -m <path>   The path to your MHA installation.  The default is useless
              except on my laptop, so you should override this.
EOF
}

while getopts hm:p: a
do
    case $a in
        m) MHA_ROOT="$OPTARG"; shift;;
        p) PLUGIN_PATH="$OPTARG"; shift;;
        h) print_help; exit;;
        \?) print_help; exit 1;;
    esac
done
shift $(expr $OPTIND - 1)

if [ ! -d "$MHA_ROOT" ]
then
    echo "Non-existent MHA root directory." >&2
    exit 1
fi

if [ ! -d "$PLUGIN_PATH" ]
then
    echo "Path to the plug-in does not exist." >&2
    exit 1
fi

PLUGIN_PATH="$(realpath $PLUGIN_PATH)"
MHA_ROOT="$(realpath $MHA_ROOT)"

export LD_LIBRARY_PATH="$PLUGIN_PATH;$MHA_ROOT/bin/"
export MHA_LIBRARY_PATH="$PLUGIN_PATH;$MHA_ROOT/bin/"

# Assume that we are using JACK2 when the executable jack_bufsize exists, then
# run it to set JACK's buffer size to what MHA expects.
if [ -x $(which jack_bufsize) ]; then
    jack_bufsize 480
fi

cd mha_config && $MHA_ROOT/bin/mha "$@" "?read:main.cfg" "cmd=start"

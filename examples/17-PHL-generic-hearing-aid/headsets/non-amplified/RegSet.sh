#!/bin/bash
set -e

function invoke_amixer {
amixer set "$C Input 1" -- "$LINPG"
amixer set "$C Input 3" -- "$RINPG"
amixer set "$C Input 2" -- "$LINNG"
amixer set "$C Input 4" -- "$RINNG"
# MX1EN and MX2EN are enabled by DAPM when PCM starts
amixer set "$C PGA Boost" -- "$LDBOOST,$RDBOOST"
amixer set "$C Aux" -- "$MX1AUXG,$MX2AUXG"
amixer set "$C" -- "$LDVOL,$RDVOL"
amixer cset name="$C Capture Switch" -- "$LDMUTE,$RDMUTE"
amixer set "$C Capture Differential" -- "$LDEN,$RDEN"
amixer set "$C Headphone" -- "$LHPVOL,$RHPVOL"
amixer cset name="$C Headphone Playback Switch" -- "$LHPM,$RHPM"
# HPEN and HPMODE are set by the driver
}

# Values for C0. "unmute" does not work, use "on" instead.

C=C0      # First Codec, Left Headset
LINPG=0   # Mute
RINPG=0   # Mute
LINNG=0   # Mute
RINNG=0   # Mute
LDBOOST=0dB
RDBOOST=0dB
MX1AUXG=0 # Mute
MX2AUXG=0 # Mute
LDVOL=30.75dB
RDVOL=30.75dB
LDMUTE=on # Unmute
RDMUTE=on # Unmute
LDEN=on   # Enable differential path
RDEN=on   # Enable differential path
LHPVOL=-10dB
RHPVOL=0dB
LHPM=on   # Unmute
RHPM=on   # Unmute

invoke_amixer

C=C1 # same settings as for C0
invoke_amixer

C=C2
LINPG=0   # Mute
RINPG=0   # Mute
LINNG=0   # Mute
RINNG=0   # Mute
LDBOOST=0dB
RDBOOST=0dB
MX1AUXG=0 # Mute
MX2AUXG=0 # Mute
LDVOL=15.75dB
RDVOL=15.75dB
LDMUTE=on # Unmute differential path
RDMUTE=on # Unmute differential path
LDEN=on   # Enable differential path
RDEN=on   # Enable differential path
LHPVOL=0dB
RHPVOL=0dB
LHPM=on   # Unmute
RHPM=on   # Unmute

invoke_amixer

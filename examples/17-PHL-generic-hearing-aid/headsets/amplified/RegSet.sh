# AMPLIFIED Headset (PGA = 0 dB)

#INPUT
# Headset inputs are taken from LINN and RINN pins and routed through
# LINNG resp. RINNG with 0dB gain.
# Unused and muted: LINPG, LDVOL, LDBOOST, RINPG, RDVOL, RDBOOST.


# # system:capture_1; Left BTE - Front Mic


# sudo i2cset -f -y 2 0x38 0x40 0x0a 0x0B i

# Bit 7 in 400a is reserved.
# Bits 4-6 in 400a set LINPG. Set to mute.
amixer set "C0 Input 1" 0
# Bits 1-3 in 400a set LINNG. Set to 0dB (= gain factor 1).
amixer set "C0 Input 2" 0dB
# Bit 0 in 400a enables left input mixer. This is done by DAPM when sound starts


# sudo i2cset -f -y 2 0x38 0x40 0x0b 0x00 i

# Bits 5-7 in 400b are reserved.
# Bits 3-4 in 400b set LDBOOST.  Set to mute.
amixer set "C0 PGA Boost" 0,
# Bits 0-2 in 400b set MX1AUXG.  Set to mute
amixer set "C0 Aux" 0,


# sudo i2cset -f -y 2 0x38 0x40 0x0e 0x00 i

# Bits 2-7 in 400e set LDVOL.  Set to lowest value.
amixer set "C0" 0,
# Bit 1 in 400e sets LDMUTE.  Set to mute.
amixer cset name="C0 Capture Switch" mute,
# Bit 0 in 400e sets LDEN. Disable.
amixer set "C0 Capture Differential" off,

# 
# # system:capture_2; Right BTE - Front Mic
# sudo i2cset -f -y 2 0x39 0x40 0x0a 0x0B i

# Bit 7 in 400a is reserved.
# Bits 4-6 in 400a set LINPG. Set to mute.
amixer set "C1 Input 1" 0
# Bits 1-3 in 400a set LINNG. Set to 0dB (= gain factor 1).
amixer set "C1 Input 2" 0dB
# Bit 0 in 400a enables left input mixer. This is done by DAPM when sound starts


# sudo i2cset -f -y 2 0x39 0x40 0x0b 0x00 i

# Bits 5-7 in 400b are reserved.
# Bits 3-4 in 400b set LDBOOST.  Set to mute.
amixer set "C1 PGA Boost" 0,
# Bits 0-2 in 400b set MX1AUXG.  Set to mute
amixer set "C1 Aux" 0,


# sudo i2cset -f -y 2 0x39 0x40 0x0e 0x00 i

# Bits 2-7 in 400e set LDVOL.  Set to lowest value.
amixer set "C1" 0,
# Bit 1 in 400e sets LDMUTE.  Set to mute.
amixer cset name="C1 Capture Switch" mute,
# Bit 0 in 400e sets LDEN. Disable.
amixer set "C1 Capture Differential" off,


# # system:capture_5; Left BTE - Rear Mic
# sudo i2cset -f -y 2 0x38 0x40 0x0c 0x0B i

# Bit 7 in 400c is reserved.
# Bits 4-6 in 400c set RINPG. Set to mute.
amixer set "C0 Input 3" 0
# Bits 1-3 in 400c set RINNG. Set to 0dB (= gain factor 1).
amixer set "C0 Input 4" 0dB
# Bit 0 in 400c enables right input mixer. This is done by DAPM when sound starts


# sudo i2cset -f -y 2 0x38 0x40 0x0d 0x00 i

# Bits 5-7 in 400d are reserved.
# Bits 3-4 in 400d set RDBOOST.  Set to mute.
amixer set "C0 PGA Boost" 0+,0
# Bits 0-2 in 400d set MX2AUXG.  Set to mute
amixer set "C0 Aux" 0+,0


# sudo i2cset -f -y 2 0x38 0x40 0x0f 0x00 i

# Bits 2-7 in 400f set RDVOL.  Set to lowest value.q
amixer set "C0" 0+,0
# Bit 1 in 400f sets RDMUTE.  Set to mute.
amixer cset name="C0 Capture Switch" ,mute
# Bit 0 in 400f sets RDEN. Disable.
amixer set "C0 Capture Differential" 0+,off


# # system:capture_6; Right BTE - Rear Mic
# sudo i2cset -f -y 2 0x39 0x40 0x0c 0x0B i

# Bit 7 in 400c is reserved.
# Bits 4-6 in 400c set RINPG. Set to mute.
amixer set "C1 Input 3" 0
# Bits 1-3 in 400c set RINNG. Set to 0dB (= gain factor 1).
amixer set "C1 Input 4" 0dB
# Bit 0 in 400c enables right input mixer. This is done by DAPM when sound starts


# sudo i2cset -f -y 2 0x39 0x40 0x0d 0x00 i

# Bits 5-7 in 400d are reserved.
# Bits 3-4 in 400d set RDBOOST.  Set to mute.
amixer set "C1 PGA Boost" 0+,0
# Bits 0-2 in 400d set MX2AUXG.  Set to mute
amixer set "C1 Aux" 0+,0


# sudo i2cset -f -y 2 0x39 0x40 0x0f 0x00 i

# Bits 2-7 in 400f set RDVOL.  Set to lowest value.
amixer set "C1" 0+,0
# Bit 1 in 400f sets RDMUTE.  Set to mute.
amixer cset name="C1 Capture Switch" ,mute
# Bit 0 in 400f sets RDEN. Disable.
amixer set "C1 Capture Differential" 0+,off


# # system:capture_3; Left Headphone Mic
# sudo i2cset -f -y 2 0x3a 0x40 0x0a 0x01 i

# Bit 7 in 400a is reserved.
# Bits 4-6 in 400a set LINPG. Set to mute.
amixer set "C2 Input 1" 0
# Bits 1-3 in 400a set LINNG. Set to mute
amixer set "C2 Input 2" 0
# Bit 0 in 400a enables left input mixer. This is done by DAPM when sound starts


# sudo i2cset -f -y 2 0x3a 0x40 0x0b 0x08 i

# Bits 5-7 in 400b are reserved.
# Bits 3-4 in 400b set LDBOOST.  Set to 0dB
amixer set "C2 PGA Boost" 0dB,
# Bits 0-2 in 400b set MX1AUXG.  Set to mute
amixer set "C2 Aux" 0,



# sudo i2cset -f -y 2 0x3a 0x40 0x0e 0x97 i

# Bits 2-7 in 400e set LDVOL.  Set to 15.75dB.
amixer set "C2" 15.75dB,
# Bit 1 in 400e sets LDMUTE.  Set to unmute.
amixer cset name="C2 Capture Switch" on,
# Bit 0 in 400e sets LDEN. Enable.
amixer set "C2 Capture Differential" on,


# # system:capture_7; Right Headphone Mic
# sudo i2cset -f -y 2 0x3a 0x40 0x0c 0x01 i

# Bit 7 in 400c is reserved.
# Bits 4-6 in 400c set RINPG. Set to mute.
amixer set "C2 Input 3" 0
# Bits 1-3 in 400c set RINNG. Set to mute
amixer set "C2 Input 4" 0
# Bit 0 in 400c enables right input mixer. This is done by DAPM when sound starts

# sudo i2cset -f -y 2 0x3a 0x40 0x0d 0x08 i

# Bits 5-7 in 400d are reserved.
# Bits 3-4 in 400d set RDBOOST.  Set to 0dB
amixer set "C2 PGA Boost" 0+,0dB
# Bits 0-2 in 400b set MX2AUXG.  Set to mute
amixer set "C2 Aux" 0+,0

# sudo i2cset -f -y 2 0x3a 0x40 0x0f 0x97 i

# Bits 2-7 in 400f set RDVOL.  Set to 15.75dB.
amixer set "C2" 0+,15.75dB
# Bit 1 in 400f sets RDMUTE.  Set to unmute.
amixer cset name="C2 Capture Switch" ,on
# Bit 0 in 400f sets RDEN. Enable.
amixer set "C2 Capture Differential" 0+,on


# #OUTPUT
# 
# # system:playback_1; Left BTE speaker
# sudo i2cset -f -y 2 0x38 0x40 0x23 0xE7 i

# Bits 2-7 in 4023 set LHPVOL.  Set to 0dB.
amixer set "C0 Headphone" 0dB,
# Bit 1 in 4023 sets LHPM. Set to unmute
amixer cset name="C0 Headphone Playback Switch" on,
# Bit 0 in 4023 sets HPEN. This is set automatically by the driver.


# # system:playback_2; Right BTE Speaker
# sudo i2cset -f -y 2 0x39 0x40 0x23 0xE7 i

# Bits 2-7 in 4023 set LHPVOL.  Set to 0dB.
amixer set "C1 Headphone" 0dB,
# Bit 1 in 4023 sets LHPM. Set to unmute
amixer cset name="C1 Headphone Playback Switch" on,
# Bit 0 in 4023 sets HPEN. This is set automatically by the driver.


# # system:playback_3; Left Headphone Speaker
# sudo i2cset -f -y 2 0x3A 0x40 0x23 0xE7 i

# Bits 2-7 in 4023 set LHPVOL.  Set to 0dB.
amixer set "C2 Headphone" 0dB,
# Bit 1 in 4023 sets LHPM. Set to unmute
amixer cset name="C2 Headphone Playback Switch" on,
# Bit 0 in 4023 sets HPEN. This is set automatically by the driver.


# # system:playback_5
# sudo i2cset -f -y 2 0x38 0x40 0x24 0xE7 i

# Bits 2-7 in 4024 set RHPVOL.  Set to 0dB.
amixer set "C0 Headphone" 0+,0dB
# Bit 1 in 4024 sets RHPM. Set to unmute
amixer cset name="C0 Headphone Playback Switch" ,on
# Bit 0 in 4024 sets HPMODE. This is set automatically by the driver.


# # system:playback_6
# sudo i2cset -f -y 2 0x39 0x40 0x24 0xE7 i

# Bits 2-7 in 4024 set RHPVOL.  Set to 0dB.
amixer set "C1 Headphone" 0+,0dB
# Bit 1 in 4024 sets RHPM. Set to unmute
amixer cset name="C1 Headphone Playback Switch" ,on
# Bit 0 in 4024 sets HPMODE. This is set automatically by the driver.


# # system:playback_7; Right Headphone Speaker
# sudo i2cset -f -y 2 0x3A 0x40 0x24 0xE7 i

# Bits 2-7 in 4024 set RHPVOL.  Set to 0dB.
amixer set "C2 Headphone" 0+,0dB
# Bit 1 in 4024 sets RHPM. Set to unmute
amixer cset name="C2 Headphone Playback Switch" ,on
# Bit 0 in 4024 sets HPMODE. This is set automatically by the driver.

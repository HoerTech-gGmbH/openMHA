#!/bin/sh


# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2024 Hörzentrum Oldenburg gGmbH
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


# Note:
# When using openMHA without the installation methods provided with the 
# distribution (i.e., binary packages or installers, respectively), the 
# following environment variables may have to be set manually:
# 1. openMHA executable (mha) must be on PATH
# 2. openMHA toolbox library (libopenmha) must be on LD_LIBRARY_PATH
# 3. openMHA plugin libraries must be on MHA_LIBRARY_PATH


# Set fragment size and sampling rate for openMHA:

# Outer fragment size / frames:
export FRAGSIZE=1024

# Sampling rate / Hz (must be equal to 16000 * RESAMPLE_RATIO):
export SRATE=48000


# Set other user-configurable openMHA configuration variables:

# Reference peak level / dB SPL for input calibration (0 dB FS corresponding 
# to 1 Pa):
export CALIB_IN_PEAKLEVEL="[93.9794]"

# Reference peak level / dB SPL for output calibration (0 dB FS corresponding 
# to 1 Pa):
export CALIB_OUT_PEAKLEVEL="[93.9794]"

# FIR filter coefficients for headphone equalization:
export CALIB_OUT_FIR="[[1]]"

# Inner fragment size for the asynchronous double buffer containing the 
# vocoder / frames (= 20 * RESAMPLE_RATIO), thus yielding a (per-electrode) 
# stimulation rate of 16000/20 pps = 800 pps and a total stimulation rate of 
# 800 * 8 pps = 6400 pps:
export DBASYNC_FRAGSIZE=60

# Delay for the asynchronous double buffer / frames (must be equal to 
# DBASYNC_FRAGSIZE - gcd(DBASYNC_FRAGSIZE, FRAGSIZE)):
export DBASYNC_DELAY=56

# Thread platform to use (for single-thread processing: "dummy"; 
# for Linux: "posix"; for Windows: "win32"):
export THREAD_PLATFORM=posix

# Scheduler used for worker threads (to the output (after prepare) of 
# "split.framework_thread_priority?"):
export WORKER_THREAD_SCHEDULER=SCHED_OTHER

# Priority assigned to worker threads (to the output (after prepare) of 
# "split.framework_thread_priority?"
export WORKER_THREAD_PRIORITY=0

# Type of stimulation:
# [[1];[0]] for only electric stimulation
# [[0];[1]] for only acoustic stimulation
# [[1];[1]] for both electric and acoustic stimulation
export STIMULATION_TYPE="[[1];[0]]"

# Time constant / s for exponentially averaged RMS:
export GET_RMS_TAU=1

# First-order Butterworth highpass filter coefficients for pre-emphasis 
# (cutoff frequency fc = 1200 Hz):
export PREEMPHASIS_IIRFILTER_B="[0.92704 -0.92704]"
export PREEMPHASIS_IIRFILTER_A="[1 -0.854081]"

# Automatic gain control parameters (fast-acting compression limiter: 
# no compression below 65 dB SPL, infinite compression above that; 
# attack time = 5 ms, decay time (i.e., release time) = 75 ms):
export DC_GTDATA="[[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -10 -11 -12 -13 -14 -15 -16 -17 -18 -19 -20 -21 -22 -23 -24 -25 -26 -27 -28 -29 -30 -31 -32 -33 -34 -35 -36 -37 -38 -39 -40 -41 -42 -43 -44 -45 -46 -47 -48 -49 -50 -51 -52 -53 -54 -55]]"
export DC_GTMIN="[0]"
export DC_GTSTEP="[1]"
export DC_TAU_ATTACK="[0.005]"
export DC_TAU_DECAY="[0.075]"

# Fourth-order Butterworth lowpass filter coefficients for anti-aliasing 
# (cutoff frequency fc = 8000 Hz):
export RESAMPLE_RATIO=3
export RESAMPLE_ANTIALIAS_B="[0.0260777 0.104311 0.156466 0.104311 0.0260777]"
export RESAMPLE_ANTIALIAS_A="[1 -1.30661 1.03045 -0.362369 0.0557639]"

# Window length / samples (= 2 * DBASYNC_FRAGSIZE/RESAMPLE_RATIO):
export WND_LEN=40

# For the ACE strategy, filterbank coefficients for analysis 
# (i.e., CI simulation) are fixed, with center frequencies fc = 
# [242 370 496 622 747 873 998 1123 1248 1432 1683 1933 2184 2493 2869 3303 3804 4364 4990 5675 6485 7421] Hz

# Number of active electrodes in an n-of-m stimulation strategy (cannot be 
# changed at runtime):
export ELECTRODOGRAM_N_ELECTRODES=8

# Compression coefficient of the loudness growth function in 
# acoustic-to-electric conversion:
export ELECTRODOGRAM_COMPRESSION_COEFFICIENT=416.2

# Base level of the input (acoustic) dynamic range / Pa (25 dB SPL):
export ELECTRODOGRAM_BASE_LEVEL=0.000355656

# Saturation level of the input (acoustic) dynamic range / Pa (65 dB SPL):
export ELECTRODOGRAM_SATURATION_LEVEL=0.0355656

# Threshold level of the output (electric) dynamic range for each 
# electrode / CU (ca. 40 dB re 1 µA at 8-bit resolution):
export ELECTRODOGRAM_THRESHOLD_LEVEL="[96]"

# Comfort level of the output (electric) dynamic range for each 
# electrode / CU (ca. 50 dB re 1 µA at 8-bit resolution):
export ELECTRODOGRAM_COMFORT_LEVEL="[160]"

# Indices of any disabled electrodes (0 = most apical, 21 = most basal):
export ELECTRODOGRAM_DISABLED_ELECTRODES="[]"

# Electrode stimulation order:
export ELECTRODOGRAM_STIMULATION_ORDER=random

# Distance of the electrodes / m:
export STIMULATION_SIGNAL_ELECTRODE_DISTANCE=0.000714286

# Length constant of exponential spread of excitation / m:
export STIMULATION_SIGNAL_LAMBDA=0.0031021

# Duration of one phase of a biphasic pulse / s:
export STIMULATION_SIGNAL_PHASE_DURATION=25e-6

# Duration of the gap between the phases of a biphasic pulse / s:
export STIMULATION_SIGNAL_INTERPHASE_GAP=8e-6

# Order of the phases of a biphasic pulse:
export STIMULATION_SIGNAL_PHASE_ORDER=cathodic_first

# Third-order gammatone filterbank coefficients for synthesis 
# (i.e., CI auralization) with center frequencies fc = 
# [713 794 913 1056 1233 1428 1663 1941 2298 2678 3152 3625 4152 4749 5363 6042 6926 8242 9643 11190 12668 14221] Hz:
export GTFB_ANALYZER_ORDER=3
export GTFB_ANALYZER0_COEFF="[(0.984465+0.0921493i)]"
export GTFB_ANALYZER0_NORM_PHASE="[2.8337e-6]"
export GTFB_ANALYZER1_COEFF="[(0.982478+0.102482i)]"
export GTFB_ANALYZER1_NORM_PHASE="[3.62421e-6]"
export GTFB_ANALYZER2_COEFF="[(0.979364+0.117606i)]"
export GTFB_ANALYZER2_NORM_PHASE="[5.03127e-6]"
export GTFB_ANALYZER3_COEFF="[(0.975317+0.135683i)]"
export GTFB_ANALYZER3_NORM_PHASE="[7.1498e-6]"
export GTFB_ANALYZER4_COEFF="[(0.969851+0.157907i)]"
export GTFB_ANALYZER4_NORM_PHASE="[1.0497e-5]"
export GTFB_ANALYZER5_COEFF="[(0.963249+0.182182i)]"
export GTFB_ANALYZER5_NORM_PHASE="[1.52296e-5]"
export GTFB_ANALYZER6_COEFF="[(0.954497+0.211126i)]"
export GTFB_ANALYZER6_NORM_PHASE="[2.25773e-5]"
export GTFB_ANALYZER7_COEFF="[(0.943034+0.244895i)]"
export GTFB_ANALYZER7_NORM_PHASE="[3.38952e-5]"
export GTFB_ANALYZER8_COEFF="[(0.926589+0.287448i)]"
export GTFB_ANALYZER8_NORM_PHASE="[5.31877e-5]"
export GTFB_ANALYZER9_COEFF="[(0.907008+0.331649i)]"
export GTFB_ANALYZER9_NORM_PHASE="[8.04245e-5]"
export GTFB_ANALYZER10_COEFF="[(0.879683+0.385056i)]"
export GTFB_ANALYZER10_NORM_PHASE="[0.000125464]"
export GTFB_ANALYZER11_COEFF="[(0.84934+0.436267i)]"
export GTFB_ANALYZER11_NORM_PHASE="[0.000184271]"
export GTFB_ANALYZER12_COEFF="[(0.8121+0.490664i)]"
export GTFB_ANALYZER12_NORM_PHASE="[0.000268142]"
export GTFB_ANALYZER13_COEFF="[(0.765814+0.548623i)]"
export GTFB_ANALYZER13_NORM_PHASE="[0.00038922]"
export GTFB_ANALYZER14_COEFF="[(0.714018+0.603873i)]"
export GTFB_ANALYZER14_NORM_PHASE="[0.000545745]"
export GTFB_ANALYZER15_COEFF="[(0.652264+0.659476i)]"
export GTFB_ANALYZER15_NORM_PHASE="[0.000760433]"
export GTFB_ANALYZER16_COEFF="[(0.565732+0.722672i)]"
export GTFB_ANALYZER16_NORM_PHASE="[0.00111191]"
export GTFB_ANALYZER17_COEFF="[(0.426695+0.796283i)]"
export GTFB_ANALYZER17_NORM_PHASE="[0.00180272]"
export GTFB_ANALYZER18_COEFF="[(0.269756+0.846403i)]"
export GTFB_ANALYZER18_NORM_PHASE="[0.00278355]"
export GTFB_ANALYZER19_COEFF="[(0.0922862+0.867124i)]"
export GTFB_ANALYZER19_NORM_PHASE="[0.00419219]"
export GTFB_ANALYZER20_COEFF="[(-0.0748154+0.853428i)]"
export GTFB_ANALYZER20_NORM_PHASE="[0.00588517]"
export GTFB_ANALYZER21_COEFF="[(-0.241042+0.805605i)]"
export GTFB_ANALYZER21_NORM_PHASE="[0.00805558]"

# Seeds for randomization of stimulation order (same value for left and 
# right: binaural link on; different values for left and right: binaural link 
# off):
export ELECTRODOGRAM_RANDOMIZATION_SEED_LEFT=1
export ELECTRODOGRAM_RANDOMIZATION_SEED_RIGHT=1

# Third-order Butterworth lowpass filter coefficients defining the extent of 
# residual acoustic hearing (cutoff frequency fc = 713 Hz):
export ACOUSTIC_IIRFILTER_B="[9.27668e-5 0.0002783 0.0002783 9.27668e-5]"
export ACOUSTIC_IIRFILTER_A="[1 -2.8134 2.64381 -0.829667]"

# Input sound file name (the file must have exactly 2 channels, and its 
# sampling rate must correspond to the value of SRATE):
export IO_IN=soundfiles/vocoder_binaural_in.wav

# Output sound file name:
export IO_OUT=vocoder_ace_binaural_out.wav


# Start the openMHA server and run the openMHA commands:
mha ?read:cfg/vocoder_ace_binaural.cfg cmd=start cmd=quit

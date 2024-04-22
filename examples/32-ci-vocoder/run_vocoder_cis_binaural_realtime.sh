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


# Prerequisites:
# 1. The current version of JACK is installed on the system
# 2. The QjackCtl executable (JACK Control audio client) is on the system PATH
# 3. A QjackCtl preset with the following specifications has been defined:
#    Preset Name:   [= value of JACK_PRESET_NAME]
#    Interface:     [= device (soundcard) to be used by JACK]
#    Sample Rate:   [= value of SRATE]
#    Frames/Period: [= value of FRAGSIZE]


# Note:
# When using openMHA without the installation methods provided with the 
# distribution (i.e., binary packages or installers, respectively), the 
# following environment variables may have to be set manually:
# 1. openMHA executable (mha) must be on PATH
# 2. openMHA toolbox library (libopenmha) must be on LD_LIBRARY_PATH
# 3. openMHA plugin libraries must be on MHA_LIBRARY_PATH


# Set preset name, fragment size, and sampling rate for JACK and openMHA, 
# respectively:

# JACK preset name:
export JACK_PRESET_NAME=vocoder_cis

# Pause length / s to allow the JACK audio server to be running before 
# continuing:
export JACK_PAUSE_LENGTH=5

# Outer fragment size / frames:
export FRAGSIZE=1024

# Sampling rate / Hz:
export SRATE=48000


# Start the JACK Control audio client:
qjackctl -s -p=${JACK_PRESET_NAME} &

# Allow the JACK audio server to be running before continuing:
clear
echo Waiting for the JACK audio server to be running...
sleep ${JACK_PAUSE_LENGTH}


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
# vocoder / frames, thus yielding a (per-electrode) stimulation rate of 
# 48000/24 pps = 2000 pps and a total stimulation rate of 
# 2000 * 12 pps = 24000 pps):
export DBASYNC_FRAGSIZE=24

# Delay for the asynchronous double buffer / frames (must be equal to 
# DBASYNC_FRAGSIZE - gcd(DBASYNC_FRAGSIZE, FRAGSIZE)):
export DBASYNC_DELAY=16

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
# (cutoff frequency fc = 1000 Hz):
export PREEMPHASIS_IIRFILTER_B="[0.938488 -0.938488]"
export PREEMPHASIS_IIRFILTER_A="[1 -0.876976]"

# Automatic gain control parameters (fast-acting compressor: 
# no compression below 52.7 dB SPL, compression with a ratio of 3:1 
# below 106 dB SPL, infinite compression above that; attack time = 4 ms, 
# decay time (i.e., release time) = 16 ms):
export DC_GTDATA="[[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -0.66667 -1.3333 -2 -2.6667 -3.3333 -4 -4.6667 -5.3333 -6 -6.6667 -7.3333 -8 -8.6667 -9.3333 -10 -10.6667 -11.3333 -12 -12.6667 -13.3333 -14 -14.6667 -15.3333 -16 -16.6667 -17.3333 -18 -18.6667 -19.3333 -20 -20.6667 -21.3333 -22 -22.6667 -23.3333 -24 -24.6667 -25.3333 -26 -26.6667 -27.3333 -28 -28.6667 -29.3333 -30 -30.6667 -31.3333 -32 -32.6667 -33.3333 -34 -34.6667 -35.3333 -36.3333 -37.3333 -38.3333 -39.3333 -40.3333 -41.3333 -42.3333 -43.3333 -44.3333 -45.3333 -46.3333 -47.3333 -48.3333 -49.3333]]"
export DC_GTMIN="[0]"
export DC_GTSTEP="[1]"
export DC_TAU_ATTACK=0.004
export DC_TAU_DECAY=0.016

# Third-order gammatone filterbank coefficients for analysis 
# (i.e., CI simulation) with center frequencies fc = 
# [120 235 384 579 836 1175 1624 2222 3019 4084 5507 7410] Hz 
# and bandwidth factor 3:
export GTFB_ANALYZER_ANALYSIS_ORDER=3
export GTFB_ANALYZER_ANALYSIS0_COEFF="[(0.987406+0.0155114i)]"
export GTFB_ANALYZER_ANALYSIS0_NORM_PHASE="[3.88027e-6]"
export GTFB_ANALYZER_ANALYSIS1_COEFF="[(0.982985+0.0302476i)]"
export GTFB_ANALYZER_ANALYSIS1_NORM_PHASE="[9.06554e-6]"
export GTFB_ANALYZER_ANALYSIS2_COEFF="[(0.976957+0.0491486i)]"
export GTFB_ANALYZER_ANALYSIS2_NORM_PHASE="[2.07418e-5]"
export GTFB_ANALYZER_ANALYSIS3_COEFF="[(0.968565+0.0735493i)]"
export GTFB_ANALYZER_ANALYSIS3_NORM_PHASE="[4.7014e-5]"
export GTFB_ANALYZER_ANALYSIS4_COEFF="[(0.956657+0.105109i)]"
export GTFB_ANALYZER_ANALYSIS4_NORM_PHASE="[0.000106197]"
export GTFB_ANALYZER_ANALYSIS5_COEFF="[(0.939524+0.145656i)]"
export GTFB_ANALYZER_ANALYSIS5_NORM_PHASE="[0.000238959]"
export GTFB_ANALYZER_ANALYSIS6_COEFF="[(0.914453+0.197378i)]"
export GTFB_ANALYZER_ANALYSIS6_NORM_PHASE="[0.000536368]"
export GTFB_ANALYZER_ANALYSIS7_COEFF="[(0.877143+0.262572i)]"
export GTFB_ANALYZER_ANALYSIS7_NORM_PHASE="[0.00120243]"
export GTFB_ANALYZER_ANALYSIS8_COEFF="[(0.821144+0.342524i)]"
export GTFB_ANALYZER_ANALYSIS8_NORM_PHASE="[0.00268248]"
export GTFB_ANALYZER_ANALYSIS9_COEFF="[(0.736802+0.436264i)]"
export GTFB_ANALYZER_ANALYSIS9_NORM_PHASE="[0.00593809]"
export GTFB_ANALYZER_ANALYSIS10_COEFF="[(0.611159+0.536963i)]"
export GTFB_ANALYZER_ANALYSIS10_NORM_PHASE="[0.0129658]"
export GTFB_ANALYZER_ANALYSIS11_COEFF="[(0.42948+0.626654i)]"
export GTFB_ANALYZER_ANALYSIS11_NORM_PHASE="[0.0277509]"

# Weights for the analysis filterbank bands:
export ELECTRODOGRAM_WEIGHTS="[1 0.752066 0.569214 0.431813 0.327594 0.248486 0.18827 0.142332 0.107405 0.0808826 0.0608164 0.0456657]"

# Compression coefficient of the loudness growth function in 
# acoustic-to-electric conversion:
export ELECTRODOGRAM_COMPRESSION_COEFFICIENT=500

# Base level of the input (acoustic) dynamic range / Pa (25 dB SPL):
export ELECTRODOGRAM_BASE_LEVEL=0.000355656

# Saturation level of the input (acoustic) dynamic range / Pa (100 dB SPL):
export ELECTRODOGRAM_SATURATION_LEVEL=2

# Threshold level of the output (electric) dynamic range for each 
# electrode / cu (ca. 36 dB re 1 µA):
export ELECTRODOGRAM_THRESHOLD_LEVEL="[60]"

# Maximum comfortable level of the output (electric) dynamic range for each 
# electrode / cu (ca. 56 dB re 1 µA):
export ELECTRODOGRAM_MAXIMUM_COMFORTABLE_LEVEL="[600]"

# Indices of any disabled electrodes (0 = most apical (low frequency), 
# 11 = most basal (high frequency)):
export ELECTRODOGRAM_DISABLED_ELECTRODES="[]"

# Electrode stimulation order:
export ELECTRODOGRAM_STIMULATION_ORDER=random

# Distance of the electrodes / m:
export STIMULATION_SIGNAL_ELECTRODE_DISTANCE=0.0024

# Length constant of exponential spread of excitation / m:
export STIMULATION_SIGNAL_LAMBDA=0.0031021

# Duration of one phase of a biphasic pulse / s:
export STIMULATION_SIGNAL_PHASE_DURATION=30e-6

# Duration of the gap between the phases of a biphasic pulse / s:
export STIMULATION_SIGNAL_INTERPHASE_GAP=2.1e-6

# Order of the phases of a biphasic pulse:
export STIMULATION_SIGNAL_PHASE_ORDER=cathodic_first

# Third-order gammatone filterbank coefficients for synthesis 
# (i.e., CI auralization) with center frequencies fc = 
# [359 548 679 945 1484 2234 3315 4592 6536 9736 12847 15641] Hz:
export GTFB_ANALYZER_SYNTHESIS_ORDER=3
export GTFB_ANALYZER_SYNTHESIS0_COEFF="[(0.991879+0.0466457i)]"
export GTFB_ANALYZER_SYNTHESIS0_NORM_PHASE="[6.93371e-7]"
export GTFB_ANALYZER_SYNTHESIS1_COEFF="[(0.988179+0.0710069i)]"
export GTFB_ANALYZER_SYNTHESIS1_NORM_PHASE="[1.5948e-6]"
export GTFB_ANALYZER_SYNTHESIS2_COEFF="[(0.985267+0.0878027i)]"
export GTFB_ANALYZER_SYNTHESIS2_NORM_PHASE="[2.53931e-6]"
export GTFB_ANALYZER_SYNTHESIS3_COEFF="[(0.978487+0.12166i)]"
export GTFB_ANALYZER_SYNTHESIS3_NORM_PHASE="[5.4631e-6]"
export GTFB_ANALYZER_SYNTHESIS4_COEFF="[(0.961242+0.189111i)]"
export GTFB_ANALYZER_SYNTHESIS4_NORM_PHASE="[1.68098e-5]"
export GTFB_ANALYZER_SYNTHESIS5_COEFF="[(0.929678+0.27989i)]"
export GTFB_ANALYZER_SYNTHESIS5_NORM_PHASE="[4.93042e-5]"
export GTFB_ANALYZER_SYNTHESIS6_COEFF="[(0.869566+0.402948i)]"
export GTFB_ANALYZER_SYNTHESIS6_NORM_PHASE="[0.00014408]"
export GTFB_ANALYZER_SYNTHESIS7_COEFF="[(0.77839+0.533774i)]"
export GTFB_ANALYZER_SYNTHESIS7_NORM_PHASE="[0.000354531]"
export GTFB_ANALYZER_SYNTHESIS8_COEFF="[(0.604693+0.696111i)]"
export GTFB_ANALYZER_SYNTHESIS8_NORM_PHASE="[0.00094634]"
export GTFB_ANALYZER_SYNTHESIS9_COEFF="[(0.259143+0.848677i)]"
export GTFB_ANALYZER_SYNTHESIS9_NORM_PHASE="[0.00285827]"
export GTFB_ANALYZER_SYNTHESIS10_COEFF="[(-0.0945864+0.849615i)]"
export GTFB_ANALYZER_SYNTHESIS10_NORM_PHASE="[0.00611443]"
export GTFB_ANALYZER_SYNTHESIS11_COEFF="[(-0.379259+0.734565i)]"
export GTFB_ANALYZER_SYNTHESIS11_NORM_PHASE="[0.0104104]"

# Seeds for randomization of stimulation order (same value for left and 
# right: binaural link on; different values for left and right: binaural link 
# off):
export ELECTRODOGRAM_RANDOMIZATION_SEED_LEFT=1
export ELECTRODOGRAM_RANDOMIZATION_SEED_RIGHT=1

# Third-order Butterworth lowpass filter coefficients defining the extent of 
# residual acoustic hearing (cutoff frequency fc = 359 Hz):
export ACOUSTIC_IIRFILTER_B="[1.23834e-5 3.71501e-5 3.71501e-5 1.23834e-5]"
export ACOUSTIC_IIRFILTER_A="[1 -2.90602 2.81641 -0.910288]"

# Input and output channels to be used by JACK:
export JACK_INPUT="[system:capture_1 system:capture_2]"
export JACK_OUTPUT="[system:playback_1 system:playback_2]"


# Start the openMHA host application and server, and run the openMHA commands 
# (close the session by entering "cmd=quit" in the command-line interface):
clear
mha --interactive ?read:cfg/vocoder_cis_binaural_realtime.cfg cmd=start

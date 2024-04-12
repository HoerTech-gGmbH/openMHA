@echo off


rem This file is part of the HörTech Open Master Hearing Aid (openMHA)
rem Copyright © 2024 Hörzentrum Oldenburg gGmbH
rem
rem openMHA is free software: you can redistribute it and/or modify
rem it under the terms of the GNU Affero General Public License as published by
rem the Free Software Foundation, version 3 of the License.
rem
rem openMHA is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem GNU Affero General Public License, version 3 for more details.
rem
rem You should have received a copy of the GNU Affero General Public License, 
rem version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.


rem Note:
rem When using openMHA without the installation methods provided with the 
rem distribution (i.e., binary packages or installers, respectively), the 
rem following environment variables may have to be set manually:
rem 1. openMHA executable (mha) must be on PATH
rem 2. openMHA toolbox library (libopenmha) must be on LD_LIBRARY_PATH
rem 3. openMHA plugin libraries must be on MHA_LIBRARY_PATH


rem Set fragment size and sampling rate for openMHA:

rem Outer fragment size / frames:
set FRAGSIZE=60

rem Sampling rate / Hz (must be equal to 16000 * RESAMPLE_RATIO):
set SRATE=48000


rem Set other user-configurable openMHA configuration variables:

rem Reference peak level / dB SPL for input calibration (0 dB FS corresponding 
rem to 1 Pa):
set CALIB_IN_PEAKLEVEL=[93.9794]

rem Reference peak level / dB SPL for output calibration (0 dB FS corresponding 
rem to 1 Pa):
set CALIB_OUT_PEAKLEVEL=[93.9794]

rem FIR filter coefficients for headphone equalization:
set CALIB_OUT_FIR=[[1]]

rem Type of stimulation:
rem [[1];[0]] for only electric stimulation
rem [[0];[1]] for only acoustic stimulation
rem [[1];[1]] for both electric and acoustic stimulation
set STIMULATION_TYPE=[[1];[0]]

rem Thread platform to use (for single-thread processing: "dummy"; 
rem for Linux: "posix"; for Windows: "win32"):
set THREAD_PLATFORM=win32

rem Scheduler used for worker threads (to the output (after prepare) of 
rem "split.framework_thread_priority?"):
set WORKER_THREAD_SCHEDULER=SCHED_OTHER

rem Priority assigned to worker threads (to the output (after prepare) of 
rem "split.framework_thread_priority?")
set WORKER_THREAD_PRIORITY=0

rem Inner fragment size for the asynchronous double buffer containing the 
rem vocoder / frames (= 20 * RESAMPLE_RATIO), thus yielding a (per-electrode) 
rem stimulation rate of 16000/20 pps = 800 pps and a total stimulation rate of 
rem 800 * 8 pps = 6400 pps:
set DBASYNC_FRAGSIZE=60

rem Delay for the asynchronous double buffer / frames (must be equal to 
rem DBASYNC_FRAGSIZE - gcd(DBASYNC_FRAGSIZE, FRAGSIZE)):
set DBASYNC_DELAY=0

rem Time constant / s for exponentially averaged RMS:
set GET_RMS_TAU=1

rem First-order Butterworth highpass filter coefficients for pre-emphasis 
rem (cutoff frequency fc = 1200 Hz):
set PREEMPHASIS_IIRFILTER_B=[0.92704 -0.92704]
set PREEMPHASIS_IIRFILTER_A=[1 -0.854081]

rem Automatic gain control parameters (fast-acting compression limiter: 
rem no compression below 65 dB SPL, infinite compression above that; 
rem attack time = 5 ms, decay time (i.e., release time) = 75 ms):
set DC_GTDATA=[[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -10 -11 -12 -13 -14 -15 -16 -17 -18 -19 -20 -21 -22 -23 -24 -25 -26 -27 -28 -29 -30 -31 -32 -33 -34 -35 -36 -37 -38 -39 -40 -41 -42 -43 -44 -45 -46 -47 -48 -49 -50 -51 -52 -53 -54 -55]]
set DC_GTMIN=[0]
set DC_GTSTEP=[1]
set DC_TAU_ATTACK=[0.005]
set DC_TAU_DECAY=[0.075]

rem Fourth-order Butterworth lowpass filter coefficients for anti-aliasing 
rem (cutoff frequency fc = 8000 Hz):
set RESAMPLE_RATIO=3
set RESAMPLE_ANTIALIAS_B=[0.0260777 0.104311 0.156466 0.104311 0.0260777]
set RESAMPLE_ANTIALIAS_A=[1 -1.30661 1.03045 -0.362369 0.0557639]

rem Window length / frames (= 2 * DBASYNC_FRAGSIZE/RESAMPLE_RATIO):
set WND_LEN=40

rem For the ACE strategy, filterbank coefficients for analysis 
rem (i.e., CI simulation) are fixed, with center frequencies fc = 
rem [242 370 496 622 747 873 998 1123 1248 1432 1683 1933 2184 2493 2869 3303 3804 4364 4990 5675 6485 7421] Hz

rem Number of active electrodes in an n-of-m stimulation strategy (cannot be 
rem changed at runtime):
set ELECTRODOGRAM_N_ELECTRODES=8

rem Compression coefficient of the loudness growth function in 
rem acoustic-to-electric conversion:
set ELECTRODOGRAM_COMPRESSION_COEFFICIENT=416.2

rem Base level of the input (acoustic) dynamic range / Pa (25 dB SPL):
set ELECTRODOGRAM_BASE_LEVEL=0.000355656

rem Saturation level of the input (acoustic) dynamic range / Pa (65 dB SPL):
set ELECTRODOGRAM_SATURATION_LEVEL=0.0355656

rem Threshold level of the output (electric) dynamic range for each 
rem electrode / CU (ca. 40 dB re 1 µA at 8-bit resolution):
set ELECTRODOGRAM_THRESHOLD_LEVEL=[96]

rem Comfort level of the output (electric) dynamic range for each 
rem electrode / CU (ca. 50 dB re 1 µA at 8-bit resolution):
set ELECTRODOGRAM_COMFORT_LEVEL=[160]

rem Indices of any disabled electrodes (0 = most apical (low frequency), 
rem 21 = most basal (high frequency)):
set ELECTRODOGRAM_DISABLED_ELECTRODES=[]

rem Electrode stimulation order:
set ELECTRODOGRAM_STIMULATION_ORDER=random

rem Distance of the electrodes / m:
set STIMULATION_SIGNAL_ELECTRODE_DISTANCE=0.000714286

rem Length constant of exponential spread of excitation / m:
set STIMULATION_SIGNAL_LAMBDA=0.0031021

rem Duration of one phase of a biphasic pulse / s:
set STIMULATION_SIGNAL_PHASE_DURATION=25e-6

rem Duration of the gap between the phases of a biphasic pulse / s:
set STIMULATION_SIGNAL_INTERPHASE_GAP=8e-6

rem Order of the phases of a biphasic pulse:
set STIMULATION_SIGNAL_PHASE_ORDER=cathodic_first

rem Third-order gammatone filterbank coefficients for synthesis 
rem (i.e., CI auralization) with center frequencies fc = 
rem [713 794 913 1056 1233 1428 1663 1941 2298 2678 3152 3625 4152 4749 5363 6042 6926 8242 9643 11190 12668 14221] Hz:
set GTFB_ANALYZER_ORDER=3
set GTFB_ANALYZER0_COEFF=[(0.984465+0.0921493i)]
set GTFB_ANALYZER0_NORM_PHASE=[2.8337e-6]
set GTFB_ANALYZER1_COEFF=[(0.982478+0.102482i)]
set GTFB_ANALYZER1_NORM_PHASE=[3.62421e-6]
set GTFB_ANALYZER2_COEFF=[(0.979364+0.117606i)]
set GTFB_ANALYZER2_NORM_PHASE=[5.03127e-6]
set GTFB_ANALYZER3_COEFF=[(0.975317+0.135683i)]
set GTFB_ANALYZER3_NORM_PHASE=[7.1498e-6]
set GTFB_ANALYZER4_COEFF=[(0.969851+0.157907i)]
set GTFB_ANALYZER4_NORM_PHASE=[1.0497e-5]
set GTFB_ANALYZER5_COEFF=[(0.963249+0.182182i)]
set GTFB_ANALYZER5_NORM_PHASE=[1.52296e-5]
set GTFB_ANALYZER6_COEFF=[(0.954497+0.211126i)]
set GTFB_ANALYZER6_NORM_PHASE=[2.25773e-5]
set GTFB_ANALYZER7_COEFF=[(0.943034+0.244895i)]
set GTFB_ANALYZER7_NORM_PHASE=[3.38952e-5]
set GTFB_ANALYZER8_COEFF=[(0.926589+0.287448i)]
set GTFB_ANALYZER8_NORM_PHASE=[5.31877e-5]
set GTFB_ANALYZER9_COEFF=[(0.907008+0.331649i)]
set GTFB_ANALYZER9_NORM_PHASE=[8.04245e-5]
set GTFB_ANALYZER10_COEFF=[(0.879683+0.385056i)]
set GTFB_ANALYZER10_NORM_PHASE=[0.000125464]
set GTFB_ANALYZER11_COEFF=[(0.84934+0.436267i)]
set GTFB_ANALYZER11_NORM_PHASE=[0.000184271]
set GTFB_ANALYZER12_COEFF=[(0.8121+0.490664i)]
set GTFB_ANALYZER12_NORM_PHASE=[0.000268142]
set GTFB_ANALYZER13_COEFF=[(0.765814+0.548623i)]
set GTFB_ANALYZER13_NORM_PHASE=[0.00038922]
set GTFB_ANALYZER14_COEFF=[(0.714018+0.603873i)]
set GTFB_ANALYZER14_NORM_PHASE=[0.000545745]
set GTFB_ANALYZER15_COEFF=[(0.652264+0.659476i)]
set GTFB_ANALYZER15_NORM_PHASE=[0.000760433]
set GTFB_ANALYZER16_COEFF=[(0.565732+0.722672i)]
set GTFB_ANALYZER16_NORM_PHASE=[0.00111191]
set GTFB_ANALYZER17_COEFF=[(0.426695+0.796283i)]
set GTFB_ANALYZER17_NORM_PHASE=[0.00180272]
set GTFB_ANALYZER18_COEFF=[(0.269756+0.846403i)]
set GTFB_ANALYZER18_NORM_PHASE=[0.00278355]
set GTFB_ANALYZER19_COEFF=[(0.0922862+0.867124i)]
set GTFB_ANALYZER19_NORM_PHASE=[0.00419219]
set GTFB_ANALYZER20_COEFF=[(-0.0748154+0.853428i)]
set GTFB_ANALYZER20_NORM_PHASE=[0.00588517]
set GTFB_ANALYZER21_COEFF=[(-0.241042+0.805605i)]
set GTFB_ANALYZER21_NORM_PHASE=[0.00805558]

rem Seed for randomization of stimulation order:
set ELECTRODOGRAM_RANDOMIZATION_SEED=1

rem Third-order Butterworth lowpass filter coefficients defining the extent of 
rem residual acoustic hearing (cutoff frequency fc = 713 Hz):
set ACOUSTIC_IIRFILTER_B=[9.27668e-5 0.0002783 0.0002783 9.27668e-5]
set ACOUSTIC_IIRFILTER_A=[1 -2.8134 2.64381 -0.829667]

rem Input file name (the file must have exactly 1 channel, and its 
rem sampling rate must correspond to the value of SRATE):
set IO_IN=soundfiles/vocoder_monaural_in.wav

rem Output sound file name:
set IO_OUT=vocoder_ace_monaural_out.wav


rem Start the openMHA server and run the openMHA commands:
mha ?read:cfg/vocoder_ace_monaural.cfg cmd=start cmd=quit
pause

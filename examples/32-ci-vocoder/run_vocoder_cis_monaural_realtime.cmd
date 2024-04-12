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


rem Prerequisites:
rem 1. The current version of JACK is installed on the system
rem 2. The QjackCtl executable (JACK Control audio client) is on the system PATH
rem 3. A QjackCtl preset with the following specifications has been defined:
rem    Preset Name:   [= value of JACK_PRESET_NAME]
rem    Interface:     [= device (soundcard) to be used by JACK]
rem    Sample Rate:   [= value of SRATE]
rem    Frames/Period: [= value of FRAGSIZE]


rem Note:
rem When using openMHA without the installation methods provided with the 
rem distribution (i.e., binary packages or installers, respectively), the 
rem following environment variables may have to be set manually:
rem 1. openMHA executable (mha) must be on PATH
rem 2. openMHA toolbox library (libopenmha) must be on LD_LIBRARY_PATH
rem 3. openMHA plugin libraries must be on MHA_LIBRARY_PATH


rem Set preset name, pause length, fragment size, and sampling rate for JACK and 
rem openMHA, respectively:

rem JACK preset name:
set JACK_PRESET_NAME=vocoder_cis

rem Pause length / s to allow the JACK audio server to be running before 
rem continuing:
set JACK_PAUSE_LENGTH=5

rem Outer fragment size / frames:
set FRAGSIZE=24

rem Sampling rate / Hz:
set SRATE=48000


rem Start the JACK Control audio client:
start qjackctl -s -p=%JACK_PRESET_NAME%

rem Allow the JACK audio server to be running before continuing:
echo Waiting for the JACK audio server to be running...
timeout /t %JACK_PAUSE_LENGTH% /nobreak > nul


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
rem "split.framework_thread_priority?"
set WORKER_THREAD_PRIORITY=0

rem Inner fragment size for the asynchronous double buffer containing the 
rem vocoder / frames, thus yielding a (per-electrode) stimulation rate of 
rem 48000/24 pps = 2000 pps and a total stimulation rate of 
rem 2000 * 12 pps = 24000 pps):
set DBASYNC_FRAGSIZE=24

rem Delay for the asynchronous double buffer / frames (must be equal to 
rem DBASYNC_FRAGSIZE - gcd(DBASYNC_FRAGSIZE, FRAGSIZE)):
set DBASYNC_DELAY=0

rem Time constant / s for exponentially averaged RMS:
set GET_RMS_TAU=1

rem First-order Butterworth highpass filter coefficients for pre-emphasis 
rem (cutoff frequency fc = 1000 Hz):
set PREEMPHASIS_IIRFILTER_B=[0.938488 -0.938488]
set PREEMPHASIS_IIRFILTER_A=[1 -0.876976]

rem Automatic gain control parameters (fast-acting compressor: 
rem no compression below 52.7 dB SPL, compression with a ratio of 3:1 
rem below 106 dB SPL, infinite compression above that; attack time = 4 ms, 
rem decay time (i.e., release time) = 16 ms):
set DC_GTDATA=[[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -0.66667 -1.3333 -2 -2.6667 -3.3333 -4 -4.6667 -5.3333 -6 -6.6667 -7.3333 -8 -8.6667 -9.3333 -10 -10.6667 -11.3333 -12 -12.6667 -13.3333 -14 -14.6667 -15.3333 -16 -16.6667 -17.3333 -18 -18.6667 -19.3333 -20 -20.6667 -21.3333 -22 -22.6667 -23.3333 -24 -24.6667 -25.3333 -26 -26.6667 -27.3333 -28 -28.6667 -29.3333 -30 -30.6667 -31.3333 -32 -32.6667 -33.3333 -34 -34.6667 -35.3333 -36.3333 -37.3333 -38.3333 -39.3333 -40.3333 -41.3333 -42.3333 -43.3333 -44.3333 -45.3333 -46.3333 -47.3333 -48.3333 -49.3333]]
set DC_GTMIN=[0]
set DC_GTSTEP=[1]
set DC_TAU_ATTACK=0.004
set DC_TAU_DECAY=0.016

rem Third-order gammatone filterbank coefficients for analysis 
rem (i.e., CI simulation) with center frequencies fc = 
rem [120 235 384 579 836 1175 1624 2222 3019 4084 5507 7410] Hz 
rem and bandwidth factor 3:
set GTFB_ANALYZER_ANALYSIS_ORDER=3
set GTFB_ANALYZER_ANALYSIS0_COEFF=[(0.987406+0.0155114i)]
set GTFB_ANALYZER_ANALYSIS0_NORM_PHASE=[3.88027e-6]
set GTFB_ANALYZER_ANALYSIS1_COEFF=[(0.982985+0.0302476i)]
set GTFB_ANALYZER_ANALYSIS1_NORM_PHASE=[9.06554e-6]
set GTFB_ANALYZER_ANALYSIS2_COEFF=[(0.976957+0.0491486i)]
set GTFB_ANALYZER_ANALYSIS2_NORM_PHASE=[2.07418e-5]
set GTFB_ANALYZER_ANALYSIS3_COEFF=[(0.968565+0.0735493i)]
set GTFB_ANALYZER_ANALYSIS3_NORM_PHASE=[4.7014e-5]
set GTFB_ANALYZER_ANALYSIS4_COEFF=[(0.956657+0.105109i)]
set GTFB_ANALYZER_ANALYSIS4_NORM_PHASE=[0.000106197]
set GTFB_ANALYZER_ANALYSIS5_COEFF=[(0.939524+0.145656i)]
set GTFB_ANALYZER_ANALYSIS5_NORM_PHASE=[0.000238959]
set GTFB_ANALYZER_ANALYSIS6_COEFF=[(0.914453+0.197378i)]
set GTFB_ANALYZER_ANALYSIS6_NORM_PHASE=[0.000536368]
set GTFB_ANALYZER_ANALYSIS7_COEFF=[(0.877143+0.262572i)]
set GTFB_ANALYZER_ANALYSIS7_NORM_PHASE=[0.00120243]
set GTFB_ANALYZER_ANALYSIS8_COEFF=[(0.821144+0.342524i)]
set GTFB_ANALYZER_ANALYSIS8_NORM_PHASE=[0.00268248]
set GTFB_ANALYZER_ANALYSIS9_COEFF=[(0.736802+0.436264i)]
set GTFB_ANALYZER_ANALYSIS9_NORM_PHASE=[0.00593809]
set GTFB_ANALYZER_ANALYSIS10_COEFF=[(0.611159+0.536963i)]
set GTFB_ANALYZER_ANALYSIS10_NORM_PHASE=[0.0129658]
set GTFB_ANALYZER_ANALYSIS11_COEFF=[(0.42948+0.626654i)]
set GTFB_ANALYZER_ANALYSIS11_NORM_PHASE=[0.0277509]

rem Weights for the analysis filterbank bands:
set ELECTRODOGRAM_WEIGHTS=[1 0.752066 0.569214 0.431813 0.327594 0.248486 0.18827 0.142332 0.107405 0.0808826 0.0608164 0.0456657]

rem Compression coefficient of the loudness growth function in 
rem acoustic-to-electric conversion:
set ELECTRODOGRAM_COMPRESSION_COEFFICIENT=500

rem Base level of the input (acoustic) dynamic range / Pa (25 dB SPL):
set ELECTRODOGRAM_BASE_LEVEL=0.000355656

rem Saturation level of the input (acoustic) dynamic range / Pa (100 dB SPL):
set ELECTRODOGRAM_SATURATION_LEVEL=2

rem Threshold level of the output (electric) dynamic range for each 
rem electrode / cu (ca. 36 dB re 1 µA):
set ELECTRODOGRAM_THRESHOLD_LEVEL=[60]

rem Maximum comfortable level of the output (electric) dynamic range for each 
rem electrode / cu (ca. 56 dB re 1 µA):
set ELECTRODOGRAM_MAXIMUM_COMFORTABLE_LEVEL=[600]

rem Indices of any disabled electrodes (0 = most apical (low frequency), 
rem 11 = most basal (high frequency)):
set ELECTRODOGRAM_DISABLED_ELECTRODES=[]

rem Electrode stimulation order:
set ELECTRODOGRAM_STIMULATION_ORDER=random

rem Distance of the electrodes / m:
set STIMULATION_SIGNAL_ELECTRODE_DISTANCE=0.0024

rem Length constant of exponential spread of excitation / m:
set STIMULATION_SIGNAL_LAMBDA=0.0031021

rem Duration of one phase of a biphasic pulse / s:
set STIMULATION_SIGNAL_PHASE_DURATION=30e-6

rem Duration of the gap between the phases of a biphasic pulse / s:
set STIMULATION_SIGNAL_INTERPHASE_GAP=2.1e-6

rem Order of the phases of a biphasic pulse:
set STIMULATION_SIGNAL_PHASE_ORDER=cathodic_first

rem Third-order gammatone filterbank coefficients for synthesis 
rem (i.e., CI auralization) with center frequencies fc = 
rem [359 548 679 945 1484 2234 3315 4592 6536 9736 12847 15641] Hz:
set GTFB_ANALYZER_SYNTHESIS_ORDER=3
set GTFB_ANALYZER_SYNTHESIS0_COEFF=[(0.991879+0.0466457i)]
set GTFB_ANALYZER_SYNTHESIS0_NORM_PHASE=[6.93371e-7]
set GTFB_ANALYZER_SYNTHESIS1_COEFF=[(0.988179+0.0710069i)]
set GTFB_ANALYZER_SYNTHESIS1_NORM_PHASE=[1.5948e-6]
set GTFB_ANALYZER_SYNTHESIS2_COEFF=[(0.985267+0.0878027i)]
set GTFB_ANALYZER_SYNTHESIS2_NORM_PHASE=[2.53931e-6]
set GTFB_ANALYZER_SYNTHESIS3_COEFF=[(0.978487+0.12166i)]
set GTFB_ANALYZER_SYNTHESIS3_NORM_PHASE=[5.4631e-6]
set GTFB_ANALYZER_SYNTHESIS4_COEFF=[(0.961242+0.189111i)]
set GTFB_ANALYZER_SYNTHESIS4_NORM_PHASE=[1.68098e-5]
set GTFB_ANALYZER_SYNTHESIS5_COEFF=[(0.929678+0.27989i)]
set GTFB_ANALYZER_SYNTHESIS5_NORM_PHASE=[4.93042e-5]
set GTFB_ANALYZER_SYNTHESIS6_COEFF=[(0.869566+0.402948i)]
set GTFB_ANALYZER_SYNTHESIS6_NORM_PHASE=[0.00014408]
set GTFB_ANALYZER_SYNTHESIS7_COEFF=[(0.77839+0.533774i)]
set GTFB_ANALYZER_SYNTHESIS7_NORM_PHASE=[0.000354531]
set GTFB_ANALYZER_SYNTHESIS8_COEFF=[(0.604693+0.696111i)]
set GTFB_ANALYZER_SYNTHESIS8_NORM_PHASE=[0.00094634]
set GTFB_ANALYZER_SYNTHESIS9_COEFF=[(0.259143+0.848677i)]
set GTFB_ANALYZER_SYNTHESIS9_NORM_PHASE=[0.00285827]
set GTFB_ANALYZER_SYNTHESIS10_COEFF=[(-0.0945864+0.849615i)]
set GTFB_ANALYZER_SYNTHESIS10_NORM_PHASE=[0.00611443]
set GTFB_ANALYZER_SYNTHESIS11_COEFF=[(-0.379259+0.734565i)]
set GTFB_ANALYZER_SYNTHESIS11_NORM_PHASE=[0.0104104]

rem Seed for randomization of stimulation order:
set ELECTRODOGRAM_RANDOMIZATION_SEED=1

rem Third-order Butterworth lowpass filter coefficients defining the extent of 
rem residual acoustic hearing (cutoff frequency fc = 359 Hz):
set ACOUSTIC_IIRFILTER_B=[1.23834e-5 3.71501e-5 3.71501e-5 1.23834e-5]
set ACOUSTIC_IIRFILTER_A=[1 -2.90602 2.81641 -0.910288]

rem Input and output channels to be used by JACK (if two output channels are 
rem specified for monaural processing, the openMHA output is presented 
rem identically on both channels):
set JACK_INPUT=[system:capture_1]
set JACK_OUTPUT=[system:playback_1 system:playback_2]


rem Start the openMHA host application and server, and run the openMHA commands 
rem (close the session by entering "cmd=quit" in the command-line interface):
cls
start /wait /b mha --interactive ?read:cfg/vocoder_cis_monaural_realtime.cfg cmd=start
pause

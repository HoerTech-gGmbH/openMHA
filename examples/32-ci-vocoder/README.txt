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


CI Vocoder Example

This example is an implementation of a cochlear implant (CI) vocoder for the 
openMHA. It is based on the original design of Bräcker et al. (2009) and a 
Matlab implementation based on this by Williges and Jürgens (2019). This openMHA 
example is a complete reimplementation which, while retaining the basic concepts 
and architecture of its predecessors, extends the previous versions with 
low-latency real-time capability and is fully integrated into the openMHA 
framework. It can therefore potentially be combined with any other existing 
openMHA plugins and configurations. The example includes:

- 6 openMHA plugins (ci_simulation_ace, ci_simulation_cis, ci_auralization_ace, 
  ci_auralization_cis, get_rms, set_rms)
  
- 14 openMHA configuration files (vocoder_*.cfg) in the directory cfg

- 8 Linux shell scripts (run_vocoder_*.sh) and 8 corresponding Windows batch 
  files (run_vocoder_*.cmd)
  
- 20 Matlab/Octave scripts (compute_*.m) and associated functions in the 
  directory mfiles

- 2 example input sound files (vocoder_*_in.wav) in the directory soundfiles

Two signal processing methods have been implemented: the ACE ("advanced 
combination encoder", "n-of-m") strategy and the CIS ("continuous interleaved 
sampling") strategy. The ACE strategy in this example shows structures and 
specifications that are based on published accounts of devices by the 
manufacturer Cochlear (e.g. Nogueira et al., 2005); similarly, the CIS strategy 
here is inspired by publicly available descriptions and specifications of MED-EL 
devices (e.g., Hochmair et al., 2006). However, none of the plugins and 
configurations here are intended to exactly mimic any existing devices. Rather, 
the selection is meant to convey an impression of the variety of signal 
processing possibilities available to the openMHA user.

The CI vocoder in this example is intended to model the implications of a CI for 
the hearing ability of a CI user in terms of speech intelligibility, pitch 
perception, sound localization, listening effort, loudness perception etc. in a 
technically and physiologically plausible way. The output of the CI vocoder 
should expressly not, however, be taken to represent the subjective auditory 
experience of CI users.


Prerequisites

The file processing configurations are designed to work "out of the box", 
generating vocoded output sound files from the example input sound files 
provided with the example (of course, other sound files can be specified by the 
user if desired). The real-time configurations, on the other hand, have 
additional prerequisites:

- The JACK Audio Connection Kit (including QjackCtl) is expected to be installed 
  and configured correctly (see the respective shell scripts and batch files for 
  details on configuring JACK for the CI vocoder).

- A sound card with a sampling rate of 48 kHz and one or two microphones (for 
  monaural or binaural processing, respectively) is required.

- Finally, some of the Matlab/Octave scripts provided with this example depend 
  on the gammatone toolbox by Hohmann and Herzke (2007), which must be 
  downloaded and then added to the Matlab/Octave search path if needed.


Usage

The CI vocoder can be used either monaurally (i.e., with a single audio input 
channel) or binaurally (i.e., with two audio input channels). What is more, it 
is capable of both offline file processing and real-time processing with one or 
more microphone input channels. It has been tested on Linux and Windows systems.

To use the CI vocoder, it is recommended to copy the entire example directory to 
a writable location on your computer. Then, any of the configurations provided 
can be started by running the corresponding shell script or batch file, 
respectively, whose names should be self-explanatory. These serve to set 
sensible parameter values and start the openMHA with the appropriate 
configurations. No further adjustments are necessary if the prerequisites 
specified above and in the respective shell scripts and batch files have been 
met.

The latency (i.e., algorithmic delay) of the CI vocoder depends on the specified 
so-called "outer" fragment size (represented by the variable FRAGSIZE). The 
minimum possible value for the outer fragment size is the so-called "inner" 
fragment size (variable DBASYNC_FRAGSIZE), which in turn depends on the desired 
(per-electrode) stimulation rate. By default, this stimulation rate is set to 
800 pps (pulses per second) for the ACE strategy and 2000 pps for the CIS 
strategy, respectively, resulting in minimum algorithmic delays 
(= FRAGSIZE/SRATE) of 1.25 ms for ACE and 0.5 ms for CIS. However, to avoid 
dropouts in the signal processing on computers with lower computing power, the 
outer fragment size is set to 1024 frames for both strategies by default, which 
results in an algorithmic delay of 21.3 ms. Matlab/Octave scripts 
(compute_timing_parameters_*.m) are provided to help users find appropriate sets 
of values for the relevant parameters if a different (e.g., smaller) outer 
fragment size is desired.

As always with the openMHA, any of the functionalities provided can in principle 
be modified. As a first step, the parameter values given in the shell scripts 
and batch files can be changed within certain limits. To make this task easier, 
Matlab/Octave scripts are provided that will generate the corresponding 
parameters exactly in the format needed for the openMHA. Users will just have to 
edit the desired values in the Matlab/Octave scripts, run those scripts, and 
copy and paste the output into the respective openMHA shell script or batch 
file. If a user-specified value happens to fall outside the range of permitted 
parameter values, an error message will indicate what must be changed.

While this will be sufficient for most users, it is of course also possible to 
change the configuration files and, thereby, the overall signal flow. This is 
particularly true if users want to make use of the possibility of adding CI 
preprocessing (e.g., beamforming), acoustic hearing impairment simulation, or 
acoustic hearing aid simulation. While the openMHA offers many possibilities for 
adding such functionalities, only the respective interfaces are included (and 
labeled as such) in this example.

Finally, it is also possible to modify the plugins themselves that form the core 
of the CI vocoder simulation and auralization. However, this is only recommended 
for users with programming experience in C++.


Architecture

The CI vocoder in this example comprises, broadly speaking, two stages: CI 
simulation and CI auralization. The first stage, CI simulation (which results in 
the electrodogram of the input signal), consists of the following processing 
steps:

- Pre-emphasis

- Automatic gain control (AGC; "single-loop" compression limiter)

- Weighted FFT-based filterbank (frequency domain; ACE only) or weighted 
  gammatone filter bank (time domain; CIS only), representing the mapping of 
  incoming sound to electrodes (channels)

- Channel envelope extraction

- Electrode selection ("n of m"; ACE only)

- Application of loudness growth function

- Definition of stimulation order (monopolar stimulation)

The second stage, CI auralization, includes the following processing steps:

- Application of inverse loudness growth function

- Spatial spread of excitation along the cochlea (channel interaction)

- Pulse generation (biphasic pulses)

- Gammatone filterbank, representing the mapping of electrodes (channels) to 
  stimulated frequencies in the cochlea

Moreover, broadband and per-channel channel RMS tracking are implemented to 
constantly control both overall and relative levels over time. Additionally, 
interfaces for I/O level calibration and headphone equalization are provided 
if needed.


Capabilities

This example can be used to model the following properties of a real CI system 
(for details, please refer to the shell scripts and batch files and the 
documentation of the respective plugins; if in doubt, please use the provided 
Matlab/Octave scripts to help you find and format appropriate values):

- Pre-emphasis parameters (filter order, cutoff frequency)

- AGC parameters (compression thresholds and ratios, time constants)

- Electrode center frequencies (CIS only)

- Disabled electrodes

- Number of active electrodes ("n of m"; ACE only)

- Parameters of the loudness growth function (acoustic and electric dynamic 
  range, compression coefficient)

- Stimulation rate (indirectly, by changing the fragment size)

- Stimulation order (random, base to apex, or apex to base)

- Properties of the biphasic pulses (cathodic first or anodic first, phase 
  duration, interphase gap)

- Electrode distance

- Properties of the spatial spread of excitation along the cochlea (length 
  constant of the exponential decay function)

- Insertion depth (indirectly, by changing the center frequencies of the CI 
  auralization filterbank)

- Frequency-to-place mismatch (automatically, by the mismatch between the center 
  frequencies of the CI simulation and CI auralization filterbanks)

- "Dead regions" of the cochlea (by setting the gain of individual filters in 
  the auralization filterbank to zero)

- Residual acoustic hearing

- Electric, acoustic, and electric-acoustic (hybrid) stimulation

- Bimodal stimulation (binaural configurations only; electric stimulation on 
  one side, acoustic stimulation on the other)

By default, the binaural configurations in this example use identical parameter 
settings on both sides, reflected by a single set of variables in the 
corresponding shell scripts and batch files. However, side-specific settings can 
easily be realized by duplicating and appropriately renaming the variables in 
the configurations for the two sides (vocoder_*_1ch_left.cfg and 
vocoder_*_1ch_right.cfg) as well as in the corresponding shell scripts and batch 
files, thus replacing, e.g., ELECTRODOGRAM_THRESHOLD_LEVEL by 
ELECTRODOGRAM_THRESHOLD_LEVEL_LEFT and ELECTRODOGRAM_THRESHOLD_LEVEL_RIGHT, 
respectively.

Also, when using random electrode stimulation order (which is the default 
setting when the vocoder output is used for auralization, because otherwise 
audible artifacts can occur at the frequency corresponding to the stimulation 
rate), the binaural link between the two sides can be turned on (same 
randomization seed for left and right; default) or off (different randomization 
seeds for left and right).

The following properties are not implemented in this example, but many of the 
corresponding functionalities can be realized with existing openMHA plugins. The 
respective configuration interfaces are provided and labeled as such in this 
example:

- Preprocessing of the electric signal (e.g., beamforming)

- Hearing impairment simulation in the acoustic hearing

- Hearing aid simulation for treatment of any hearing impairment in the 
  acoustic hearing


Applications

Possible applications of this openMHA CI vocoder include (but are not 
necessarily limited to):

- Psychoacoustic research (on fields like, e.g., speech intelligibility, pitch 
  perception, sound localization, listening effort, loudness perception etc.) 
  with normal-hearing, simulated CI users

- Development and evaluation of new stimulation (sound coding) strategies, 
  including the generation of the actual pulse patterns for real research CIs 
  via manufacturers' research interfaces

- Speech therapy

- Teaching and awareness raising

The configurations in this example offer many interfaces (including predefined 
AC variables, e.g., for the electrodogram) that allow for flexibility in many 
possible settings. Note, however, that some of these applications may still 
require modifications or additions to the system in terms of plugins, 
configurations, user interfaces, etc.


Funding

This work was funded by the EU (ERDF) as part of the VIBHear research project.


Acknowledgments

We would like to thank Ben Williges for his extremely helpful cooperation and 
willingness to share his expertise in the development of the CI vocoder.


References

Bräcker, T., Hohmann, V., Kollmeier, B., & Schulte, M. (2009). Simulation und 
  Vergleich von Sprachkodierungsstrategien in Cochlea-Implantaten. Zeitschrift 
  für Audiologie, 48(4), 158-169. 
  https://www.z-audiol.de/index.php/archiv/originalartikel/2009_04_original_158-169

Hochmair, I., Nopp, P., Jolly, C., Schmidt, M., Schößer, H., Garnham, C., & 
  Anderson, I. (2006). MED-EL cochlear implants: State of the art and a glimpse 
  into the future. Trends in Amplification, 10(4), 201-220. 
  https://doi.org/10.1177/1084713806296720

Hohmann, V., & Herzke, T. (2007). Software for "Frequency analysis and synthesis 
  using a Gammatone filterbank" (Version 1.1). Zenodo. 
  https://doi.org/10.5281/zenodo.2643400

Nogueira, W., Büchner, A., Lenarz, T., & Edler, B. (2005). A psychoacoustic 
  "NofM"-type speech coding strategy for cochlear implants. EURASIP Journal on 
  Advances in Signal Processing, 2005, Article 101672. 
  https://doi.org/10.1155/ASP.2005.3044

Williges, B., & Jürgens, T. (2019). Pulsatile cochlear implant vocoder 
  (Version 1.0.1). Zenodo. https://doi.org/10.5281/zenodo.3234499

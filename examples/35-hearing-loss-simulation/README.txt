# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2017 2018 2019 2020 2021 HörTech gGmbH
# Copyright © 2023 2024 2025 Hörzentrum Oldenburg gGmbH
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


Hearing Loss Simulation Example

This example is an implementation of a hearing loss, or hearing impairment, 
simulator (HSIM) for the openMHA. It is an extension of the version included in 
the previous release of the openMHA. While retaining its basic architecture, 
low-latency real-time capability, and full integration into the openMHA 
framework, it offers more flexibility, configurability, and usability, including 
a number of Matlab/Octave helper functions for computing, setting, changing, and 
plotting the various parameters. The example includes:
  
- 3 openMHA configuration files (HSIM_*.cfg) in the directory cfg

- 3 Linux shell scripts (run_HSIM_*.sh) and 3 corresponding Windows batch 
  files (run_HSIM_*.cmd)
  
- 4 Matlab/Octave functions (plot_HSIM_parameters.m, set_HSIM_parameters_*.m) 
  in the directory mfiles

- 1 example input sound file (HSIM_in.wav) in the directory soundfiles

The configurations provided in this example support real-time signal processing 
(HSIM_live.cfg), sound file processing (HSIM_files.cfg), and real-time looped 
processing of a sound file (HSIM_files_live.cfg).

When run with the default settings, the configurations distributed with this 
example implement the simulation of a hearing impairment of type N1 (i.e., a 
very mild, flat / moderately sloping hearing loss) according to Bisgaard et al. 
(2010), with corresponding impaired loudness growth curves and loss of 
spectro-temporal resolution, using somewhat simplified assumptions based on the 
literature (see below). A large number of parameters can be adjusted, either 
manually or by using the presets defined in the accompanying Matlab/Octave 
tools.


Prerequisites

The file processing configuration is designed to work "out of the box", 
generating an output sound file with simulated hearing loss from the example 
input sound file provided with this example (of course, other sound files can be 
specified by the user if desired). The real-time configurations (for processing 
either looped sound files or microphone signals), on the other hand, have 
additional prerequisites:

- The JACK Audio Connection Kit (including QjackCtl) is expected to be installed 
  and configured correctly (see the respective configuration files for details 
  on configuring JACK for the hearing loss simulator).

- An audio interface (sound card) with a sample rate and buffer size set to the 
  same values as JACK is required.

Furthermore, to be able to use this hearing loss simulator correctly, 
flexibly, and reliably, note the following requirements:

- Calibration of the input levels (via calib_in.peaklevel) and output levels 
  (via calib_out.peaklevel) is necessary to ensure that the level-dependent 
  algorithms in the example (specifically, the dynamic range expansion 
  algorithm) work as expected and that the attenuation effects of a given 
  hearing loss are presented realistically. (When using the default calibration 
  values, *input* calibration is not strictly necessary for file-to-file 
  processing and real-time looped file processing, because the absolute sound 
  pressure level of the input signal can be set with the Matlab/Octave tools 
  distributed with this example.) Please refer to the openMHA calibration 
  instructions manual for details on how to perform the calibration.

- It is strongly recommended to install Matlab or Octave to be able to run the 
  helper tools used to set, change, and/or plot the parameters of the simulator. 
  While it is possible to change the parameters directly in the configuration 
  files or interactively in a running openMHA instance, this will ususally be 
  neither necessary nor practical. All functions run in both Matlab and Octave; 
  however, for some of the algorithms, Matlab is much faster than Octave.

- The Matlab/Octave functions provided with this example depend on a gammatone 
  toolbox (Hohmann, 2002; Hohmann & Herzke, 2007), which must be downloaded and 
  then added to the Matlab/Octave search path. Also, the mfiles directory from 
  the openMHA installation (which contains additional helper functions) must be 
  on the search path.

- Finally, when running the real-time configurations of the hearing loss 
  simulator in *Windows*, care must be taken to ensure that the operating system 
  continually allocates sufficient computational resources to the process. While 
  this will be highly system-dependent, it may for example be necessary to 
  change the power options in Windows (temporarily) such that the minimum 
  processor state is set to 100 %.

Note that when performing real-time processing of *microphone signals*, due to 
the unavoidable leakage of direct sound through the headphones it will usually 
be necessary for the speaker and listener to be in separate rooms for the 
hearing loss simulation to be used meaningfully.


Usage

The hearing loss simulator works with stereo signals (one channel for each 
ear) - either with stereo sound files or with two microphone signals. As 
mentioned above, it is capable of both offline file processing, real-time looped 
file processing, and real-time processing with microphone input channels. It 
has been tested on Linux and Windows systems.

To use the hearing loss simulator, it is recommended to copy the entire example 
directory to a writable location on your computer. Then, the following steps 
must be performed:

1. Start QjackCtl with the correct preset (real-time processing only).

2. Start openMHA in interactive mode and read the correct configuration file 
   (the shell scripts / batch files distributed with this example can be used 
   for this purpose).

3. Set the desired parameters with Matlab/Octave (optional).

4. Run cmd=start in openMHA.

5. Change the parameters with Matlab/Octave as desired (optional; real-time 
   processing only).

6. Run cmd=stop (or cmd=quit to end the session) in openMHA.

As always with the openMHA, any of the functionalities provided can in principle 
be modified. As a first step, the parameter values given in the configuration 
files can be changed within certain limits. To make this task easier, 
Matlab/Octave functions are provided that will generate the corresponding 
parameters exactly in the format needed for the openMHA. Users will just have to 
edit the desired values in the Matlab/Octave functions and then run those 
functions; the openMHA parameters are then computed and sent to the running 
openMHA instance via a TCP network connection. If a user-specified value happens 
to fall outside the range of permitted parameter values, an error message will 
indicate what must be changed. - While this will be sufficient for most users, 
it is of course also possible to change the configuration files themselves and, 
thereby, the overall architecture and signal flow of the simulator.


Architecture

The following signal processing method has been implemented: The signal is first 
split into individual frequency bands; then, a level-dependent gain (i.e., 
dynamic range expansion) is applied to each band to simulate threshold 
elevation, recruitment, and a reduced dynamic range due to the hearing 
impairment; finally, band-wise spectral smearing is performed to simulate 
amplitude uncertainty, reduced frequency selectivity, and temporal jitter 
resulting from the impaired cochlea. Thus, both the *attenuation* and the 
*distortion* component of hearing loss are simulated. More specifically, the 
following steps are performed for each input signal frame (see the comments in 
the configuration files for more details):

- First, each channel of the 2-channel (i.e., stereo) audio signal is filtered 
  in a 32-band time-domain (gammatone) filter bank, resulting in a total of 
  2 * 32 = 64 bands.

- Next, a dynamic range expansion gaintable is used to multiply the signals in 
  each of the 64 bands with a (frequency- and level-dependent) gain. The 
  hearing threshold levels from the specified audiogram are used to simulate 
  threshold elevation, while including the corresponding most comfortable 
  and uncomfortable loudness levels serves to simulate recruitment and a reduced 
  dynamic range.

- Then, the (expanded) signals in each band are multiplied with (uncorrelated) 
  low-pass filtered white noise (Hülsmeier et al., 2020; Mourgela et al., 2020), 
  such that the cutoff frequency of the filter corresponds to the *smearing 
  width*, that is, the width by which the auditory filter at a given frequency 
  is assumed to be broadened by hearing loss. (Note that the *smearing factor*, 
  in contrast, refers to the *relative* broadening of each auditory filter; 
  i.e., a smearing factor of 1 corresponds to a smearing width of 0 for a 
  normal-hearing auditory filter.) Since multiplication in the time domain 
  corresponds to convolution in the frequency domain, the multiplicative 
  narrowband noise not only has the effect of introducing amplitude uncertainty 
  (i.e., reduced speech intelligibility) and temporal jitter (i.e., impaired 
  pitch perception), but also reduced frequency selectivity due to the broader 
  auditory filters.

- Finally, all 32 bands of each channel are recombined by summation, again 
  resulting in a 2-channel broadband signal.


Capabilities

This example can be used to model the following properties of hearing impairment 
(for further details, please refer to the comments in the respective 
configuration files; if in doubt, please use the provided Matlab/Octave 
functions to help you find and format appropriate values):

- Threshold elevation is directly introduced via the (air-conduction) hearing 
  threshold levels (HTL) of the input audiogram. This audiogram can either be 
  specified manually (with arbitrary frequency resolution) or chosen from a set 
  of standard audiogram types according to Bisgaard et al. (2010). The values 
  for each audiogram frequency are subsequently mapped to the center frequencies 
  of the auditory filterbank by interpolation. Normal-hearing thresholds can be 
  chosen for reference; in that case, no changes to the input signal are made, 
  except that the dynamic range is explicitly limited to the normal-hearing 
  dynamic range (between hearing threshold level and uncomfortable loudness 
  level) for direct comparison with the simulated impaired signals.

- Recruitment (i.e., a steeper-than-normal loudness growth curve) results from 
  the way the (frequency-specific) input-output characteristic of the dynamic 
  range expansion algorithm is constructed. From the given audiogram thresholds 
  (see above), appropriate most comfortable (MCL) and uncomfortable loudness 
  levels (UCL) are computed based on the data reported by Pascoe (1988), fitting 
  a cubic polynomial function to the data to produce a smooth, monotonic 
  function. Alternatively, MCL and UCL can also be specified manually if the 
  data are available. The loudness growth curve is then defined using the HTL, 
  MCL, and UCL as support points (after converting the data from dB HL to dB SPL 
  using the equal-loudness contour at threshold from ISO 226:2023). Below the 
  HTL, a noise gate is defined, while above the UCL, a limiter is implemented.

- Reduced dynamic range, like recruitment, results directly from the 
  specification of the HTL and UCL for any given frequency.

- Amplitude uncertainty in the simulated impaired signal is a consequence of 
  multiplying the input signal with narrowband white noise, which changes the 
  temporal envelope of the signal. As explained above, the bandwidth of the 
  noise depends on the (normal-hearing) bandwidth of the respective auditory 
  filter and the smearing factor (i.e., the broadening factor of the auditory 
  filter), which in turn depends on the degree of hearing loss. The assumption 
  made here is that for any given frequency, the smearing factor is a well-
  defined function of the HTL at that frequency. To estimate that function, 
  measured values of that relationship from the literature for various auditory 
  filter center frequencies (Glasberg & Moore, 1986; Peters & Moore, 1992) were 
  inspected; as a first approximation, the data for 1000 Hz was chosen as a 
  basis for this simulator. A cubic polynomial function is then fitted to that 
  data. For HTL values below 0 dB HL, the smearing factor is fixed at 1.0, while 
  (theoretical) smearing factor values above the estimated value corresponding 
  to a completely passive cochlea are limited to that value (approximately 3.8; 
  Moore & Glasberg, 2004).

- Reduced frequency selectivity, as explained above, is also a consequence of 
  the time-domain multiplication with narrowband white noise, which corresponds 
  to a convolution in the frequency domain and thus effectively broadens the 
  auditory filters and reduces their spectral resolution (Baer & Moore, 1993).

- Temporal jitter, finally, is also due to the multiplicative noise, resulting 
  in degraded pitch perception.

For more complex impairment and rehabilitation scenarios, it is possible to 
combine this hearing loss simulator with other openMHA configurations, like, for 
instance, a generic hearing aid (example 4) and/or a CI vocoder (example 32). 
Some knowledge about how to create and modify openMHA configurations will be 
necessary for such purposes.

At present, the configurations in this example are limited to simulating the 
attenuation and distortion components of *sensorineural* hearing losses (Plomp, 
1978). In the future, it would be desirable to add the option of *conductive* 
(and, thereby, also *mixed*) hearing losses. Such hearing impairments are 
characterized by a significant difference between air-conduction and bone-
conduction thresholds in the audiogram; in terms of the dynamic range expansion 
gaintables used in this simulator, this would effectively result in the input-
output characteristic being shifted to the *right* (by a frequency-dependent 
value), with the curve otherwise largely unchanged.

Also, the simulated hearing impairment so far only includes *monaural* effects 
of hearing impairment. The modeling of *binaural* effects, such as for example 
binaural loudness summation or interaural correlation of the distortion 
component, are, again, in principle possible but as yet not implemented.


Applications

Possible applications of this openMHA hearing loss simulator include (but are 
not necessarily limited to):

- Psychoacoustic research (on fields like, e.g., speech intelligibility, pitch 
  perception, listening effort, loudness perception, etc.) with normal-hearing, 
  simulated hearing-impaired users

- Teaching (e.g. in audiology or speech therapy)

- Public awareness raising


Funding

This work was funded by the Deutsche Forschungsgemeinschaft (DFG, German 
Research Foundation) under Germany's Excellence Strategy - EXC 2177/1 - 
Project ID 390895286.


References

Baer, T., & Moore, B. C. J. (1993). Effects of spectral smearing on the 
  intelligibility of sentences in noise. The Journal of the Acoustical Society 
  of America, 94(3), 1229-1241. https://doi.org/10.1121/1.408176

Bisgaard, N., Vlaming, M. S. M. G., & Dahlquist, M. (2010). Standard audiograms 
  for the IEC 60118-15 measurement procedure. Trends in Amplification, 14(2), 
  113-120. https://doi.org/10.1177/1084713810379609

Glasberg, B. R., & Moore, B. C. (1986). Auditory filter shapes in subjects with 
  unilateral and bilateral cochlear impairments. The Journal of the Acoustical 
  Society of America, 79(4), 1020-1033. https://doi.org/10.1121/1.393374

Glasberg, B. R., & Moore, B. C. (1990). Derivation of auditory filter shapes 
  from notched-noise data. Hearing research, 47(1-2), 103-138. 
  https://doi.org/10.1016/0378-5955(90)90170-t

Hohmann, V. (2002). Frequency analysis and synthesis using a Gammatone 
  filterbank. Acta Acustica united with Acustica, 88(3), 433-442.

Hohmann, V., & Herzke, T. (2007). Software for "Frequency analysis and synthesis 
  using a Gammatone filterbank" (Version 1.1). Zenodo. 
  https://doi.org/10.5281/zenodo.2643400

Hülsmeier, D., Warzybok, A., Kollmeier, B., & Schädler, M. R. (2020). 
  Simulations with FADE of the effect of impaired hearing on speech recognition 
  performance cast doubt on the role of spectral resolution. Hearing Research, 
  395, Article 107995. https://doi.org/10.1016/j.heares.2020.107995

International Organization for Standardization (ISO). (2023). Acoustics - Normal 
  equal-loudness-level contours (ISO 226:2023). 
  https://www.iso.org/standard/83117.html

Moore, B. C., & Glasberg, B. R. (2004). A revised model of loudness perception 
  applied to cochlear hearing loss. Hearing research, 188(1-2), 70-88. 
  https://doi.org/10.1016/S0378-5955(03)00347-2

Mourgela, A., Agus, T. R., & Reiss, J. D. (2020, October 27-30). Investigation 
  of a real-time hearing loss simulation for use in audio production 
  [Engineering brief 620]. 149th Audio Engineering Society Convention.
  https://aes2.org/publications/elibrary-page/?id=20906

Pascoe, D. P. (1988). Clinical measurements of the auditory dynamic range and 
  their relation to formulas for hearing aid gain. In J. Hartvig Jensen (Ed.), 
  Hearing aid fitting: Theoretical and practical views. Proceedings of the 13th 
  Danavox Symposium (pp. 129-152). Danavox Jubilee Foundation.

Peters, R. W., & Moore, B. C. (1992). Auditory filter shapes at low center 
  frequencies in young and elderly hearing-impaired subjects. The Journal of the 
  Acoustical Society of America, 91(1), 256-266. https://doi.org/10.1121/1.402769

Plomp, R. (1978). Auditory handicap of hearing impairment and the limited 
  benefit of hearing aids. The Journal of the Acoustical Society of America, 
  63(2), 533-549. https://doi.org/10.1121/1.2015802

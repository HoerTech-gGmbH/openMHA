# Application Note
## Calibrating openMHA on the BatAndCat Portable Hearing Lab

### Introduction
This application note describes a possible method for calibrating openMHA on
the Bat&Cat portable hearing lab (PHL) with behind-the-ear (BTE) headsets
containing receivers in the canal (RIC).
Please read our calibration manual before following this application note.
As stated in the calibration manual, specialized measurement hardware and a
thorough understanding of hearing aid calibration is required for following
the calibration manual and this application note.  In this application
note, we use hardware from Bruel & Kjaer (B&K) and GRAS.
While we provide some scripts that will help you with the calibration of
openMHA on the PHL, we also ask you to inspect these scripts before applying
them and apply your own judgement if the procedure and settings in these
scripts are appropriate for your setup or if you need to change anything.

We cannot guarantee that the procedures presented here are free of errors.
If you discover something that is wrong, please get in contact with the openMHA
project, e.g. by filing an issue with our github project.
We will do our best to improve the documentation and the tools that we provide
as we go along.
The method described here roughly follows Denk et al. (2018):
Denk, F., Ernst, S. M. A., Ewert, S. D., & Kollmeier, B. (2018).
Adapting Hearing Devices to the Individual Ear Acoustics:
Database and Target Response Correction Functions for Various Device Styles.
Trends in Hearing. https://doi.org/10.1177/2331216518779313

Because RICs are normally used in open fittings
which cannot be used to amplify sounds at low frequencies,
we will only calibrate this setup for frequencies above 1kHz. 

Where this application note refers to Octave, users can use Matlab as a
replacement.

### Calibrating PHL Microphones
To calibrate all 4 microphones of the BTE headset, we produce a speech
simulation noise, remove the frequencies below 1kHz from this noise,
and present the residual noise with a speaker at 80 dB SPL.

File `generate_mic_calib_noise.cfg` contains an openMHA
configuration file that can be used to produce suitable noise on a computer
with openMHA installed:

* Inspect the file's contents to see if you need to adjust any settings to
  your setup.
* Start a Jack sound server with 48kHz sampling rate and a block size of 2048
  samples.
* Start an openMHA instance with this configuration.
* Adjust the sound level so that your level meter reads 80 dB SPL at the
  measurement position. Ensure that the sound is not clipped or distorted.
* Replace your level meter with an artificial head, place the BTE headsets
  on the head's ears, and connect them to the already switched-on
  PHL box running the latest mahalia SD card image.
* While the noise is produced and the PHL is running, connect a computer
  running Octave to the wireless network created by the running PHL box and
  execute the following script in Octave after inspecting it to see if you
  need to apply any changes: `adjust_peaklevel_in.m`

While your speaker produces the calibration noise, execute this script to
query the openMHA instance running on the PHL for the current input level.
It will then adjust the input peaklevel and reread the input levels
to determine if the adjustment had the desired effect.

Result of this section are adjusted input peaklevel parameter values.
The script will apply the new peaklevel values to the running openMHA instance,
but the values will be lost when openMHA starts next on the same device.

Write down the values and reapply them every time an openMHA instance starts on
and with the same hardware.

We will provide a way to store the new peaklevel values permanently on the device
in the future, but this feature has not yet been developed.

### Calibrating PHL outputs (RICs)

Here we use an IEC 711 ear simulator (B&K 4157) to measure the frequency
response of the RICs.

Switch on PHL Box with latest mahalia image installed.

Connect the B&K 4157 to a sound card line input (requires microphone
preamplifier B&K 2669, GRAS Power Module 12AA, and correct cables).

Attach adapter B&K DB 2015 to B&K 4157 with B&K DP 0530. Insert the attached
adapter B&K DB 2015 into calibrator B&K 4231 and switch the calibrator on.

The computer connected to the sound card must also be running Octave,
have openMHA, Jack and jack_playrec
(https://github.com/HoerTech-gGmbH/jack_playrec/) installed, and be connected
to the wireless or usb network provided by the PHL box. Inspect the Octave
function

`adjust_output_calibration.m`

to see if you need to make any changes to this script, then
start a Jack server with 48000 Hz sampling rate and execute the script.
Follow the instructions, which will include detaching the calibrator
and attaching each of the RICs in turn to the B&K DB 2015 with suitable
material.

Invoke the Octave function in such a way that you capture both return
values in workspace variables, e.g. 
`[peaklevel, gains]=adjust_output_calibration`. 
The function will record and analyze sounds captured by the
measurement microphone inside the B&K 4157, and compute the necessary
output peaklevel and filter to calibrate the output side of the PHL.

In order to do this, it will also start an openMHA istance locally
and analyze its output as well as the free-field-to-eardrum transfer
function from Denk et al. 2018.

Results of this procedure will be a set of peaklevel parameter values
for the two RICs and a matrix of equalizer gains to be applied in the
STFT domain of the basic hearing aid setup running on the PHL.
The function will apply the peaklevel and equalizer gains to the running
openMHA instance, but the values will not be applied automatically when
openMHA starts next on the same device.

Write down or store the values and reapply them every time an openMHA
instance starts on and with the same hardware.

We will provide a way to store the new peaklevel and gain values
permanently on the device in the future, but this feature has not yet been
developed.

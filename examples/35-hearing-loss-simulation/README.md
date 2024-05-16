## Hearing Loss Simulation

This directory contains two configuration files for hearing loss simulation
with openMHA, one for live signal processing, one for sound file processing.

Both configurations should be inspected and modified before use to suit your
needs.

The signal is split into 32 gammatone frequency bands, a level-dependent
gain is applied to each band to perform the hearing loss level simulation,
then the signal is processed with spectral smearing to simulate the reduced
frequency selectivity of the impaired cochlea.

The following changes should be made before use:
The system needs to be calibrated to the microphones and headphones used.
This is important because hearing loss simulation is non-linear and level-
dependent. Set parameters mha.calib_in.peaklevel and mha.calib_out.peaklevel.
Check the hints in the configuration files and for more information, refer
to the openMHA calibration manual.

The distributed files simulate a sloping hearing loss from 30dB HL at
low frequencies to 70dB HL at high frequencies. Adjust the gain table
set in mha.c.split_dc.c.dc.gtdata to simulate the desired hearing loss,
see explanation in the configuration files.

In order to control the spectral smearing, change the settings in
mha.c.split_smearing.s.*.lpfilt.* and ha.c.split_smearing.s.*.smeared.out.
See comments in the configuration files for more information.

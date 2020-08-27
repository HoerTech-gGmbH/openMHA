This is a minimal example on  how to use the openMHA for a simple audio processing task
for both live and file-to-file audio processing.

The task demonstrated is the application of a gain to an audio input signal. The file
gain.cfg shows how to do file-to-file processing with the MHAIOFile plugin, gain_live.cfg
and gain_live_double.cfg demonstrate usage of the MHAIOJack plugin, the latter with the
added complexity of double buffering between the Jack audio server and the audio processing.

The configuration files are extensively documented. The usage of this example is demonstrated
step-by-step in the openMHA_starting_guide.pdf.

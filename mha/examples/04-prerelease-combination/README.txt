The configuration file in this directory, prerelease_combination.cfg,
combines plugins for calibration, adaptive differential microphones,
STFT processing, coherence filtering, a filterbank, and dynamic compression.

The configuration file contains extensive documentation. Please refer
to the comments in the configuration file to learn more about this
combination of algorithms.

The configuration performs audio file processing: It processes the sound
file 1speaker_diffNoise_4ch.wav in this directory and produces the output
sound file 1speaker_diffNoise_4ch_OUT.wav.

The processing is started like this:
mha ?read:prerelease_combination.cfg cmd=start cmd=quit

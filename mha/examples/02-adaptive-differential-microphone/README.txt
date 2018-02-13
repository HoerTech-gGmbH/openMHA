The configuration file in this directory, adm.cfg,
demonstrates how to use the plugin "adm", which combines two omnidirectional
microphones on one hearing aid device to one adaptive differential microphone.

The configuration file contains extensive documentation. Please refer
to the comments in the configuration file to learn more about this
algorithm.

The configuration performs audio file processing: It processes the sound
file 1speaker_diffNoise_4ch.wav in this directory and produces the output
sound file 1speaker_diffNoise_4ch_OUT.wav.

Note that the output sound file contains only 2 channels, because the algorithm
combines the 4 input channels into 2 output channels.

The processing is started like this:
mha ?read:adm.cfg cmd=start cmd=quit

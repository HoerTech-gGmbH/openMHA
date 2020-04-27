This directory contains two examples that illustrate the usage of the split
plugin. This plugin splits the audio signal into groups of one or more
channels each and processes them in parallel.

This has two main use cases:

1) Hide some channels from a plugin that does not have the capability
to selectively process channels. 
This is demonstrated in 'split_simple.cfg'. As the noise plugin adds noise to every 
channel of its input, it is not possible to selectively add noise only to some channels.
 This configuration splits a stereo signal into to groups of one channel and adds noise to one group
and passes through the other.

2) Parallelize computationally expensive operations, demonstrated in "split_live_posix.cfg" respective
"split_live_win32.cfg".
In these examples we use live input output, imposing an upper bound on the allowed
processing time per audio buffer, aka real time processing.
By default, all audio processing is done in a single thread utilizing one cpu core.
Using the split plugin, we can spawn additional threads, using multiple cpu core in
parallel.

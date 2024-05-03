This example demonstrates different hearing aid signal enhancement algorithms
implemented in openMHA that run in real-time. 

The following methods are available:

Two classical spatial filtering algorithms:
- bilateral Adaptive Differential Microphones (ADM)
- binaural Minimum-Variance Distortionless-Response (MVDR) beamformer 

and three deep neural network (DNN)-based approaches:
- monaural group communication filter-and-sum network (GCFSnet_mono)
- binaural group communication filter-and-sum network (GCFSnet_bin)
- binaural Multi-Frame Wiener Filter (bMFWF) 

A binaural behind-the-ear (BTE) hearing aid setup is assumed with two
microphones (10mm distance) on each ear such as the ear-level devices available
with the Portable Hearing Lab (see http://www.openmha.org/hardware/). 

This example uses a JACK server for live audio input and output. Internally,
the sampling rate of the algorithms is 16kHz, but for the outer sampling rate
48kHz is used to achieve compatibility with regular audio hardware.

To run this example start a JACK server with 4 input channels and 2 output
channels, a sample rate of 48000 Hz and 96 frames/period.

Then, execute the following command in your terminal: 

export OMP_NUM_THREADS=1; mha '?read:index.cfg'  


(export OMP_NUM_THREADS=1 is used since it turned out to be advantageous
to limit the number of cores used for the execution to one, otherwise the
multi-threading overhead can overload the system.)

You can switch between the different algorithms at runtime by selecting one
of the available options:
mha.transducers.mhachain.signal_enhancement.select = {pass ADM MVDR GCFSnet_mono GCFSnet_bin bMFWF}

where 'pass' is the default setting that does not aply any signal enhancement.

Notes regarding the DNN-based algorithms:
- These are experimental algorithms used to demonstrate the capability of
  openMHA to running deep-learning based speech enhancement algorithms in real
  time with low latency.
- The DNN-based algorithms may be sensitive to the absolute sound level and
  therefore require a calibrated setup to work properly. Modify the calibration
  parameters in index.cfg (l 9-10) accordingly.
- Please note that the DNN-based algorithms may cause high CPU-load. In
  particular the bMFWF requires a high processing performance and may not run
  properly on some computers.

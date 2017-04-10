# openMHA

HörTech Open Master Hearing Aid (openMHA) 

1. Content of the Pre-Release 

The software contains the source code of the openMHA Toolbox library, of the 
openMHA framework and command line application, and of a selection of algorithm 
plugins forming a basic hearing aid processing chain featuring
- calibration
- bilateral adaptive differential microphones for noise suppression
- binaural coherence filter for feedback reduction and dereverberation
- multi-band dynamic range compressor for hearing loss compensation


2. Source Code Overview 

Together with the MHA source code, we deliver the source code for the FFTW 
library, version 2.1.5, below the directory external_libs.
All openMHA source code lives under the directory mha: The MHA Toolbox
Library can be found in the sub-directory mha/libmha. It contains the
implementation of the MHA configuration language, of signal processing
primitives and also many signal processing algorithms to by used in MHA
plugins. The MHA command line application and its support framework 
can be found in the sub-director mha/frameworks. The plugins contained in this 
pre-release can be found in the sub-directory mha/plugins. The IO libraries, 
that connect the MHA e.g. to the sound card for live processing, or to sound 
files for offline processing, are also found here. 


3. Build Instructions 

The MHA source code has to be compiled before the MHA can be used. While MHA in 
general can be compiled for many operating systems and hardware platforms, in 
this pre-release we concentrate on compilation on Ubuntu 16.04 for 64-bit 
processors (x86_64). 

Prerequisites: 
64-bit version of Ubuntu 16.04 or later, 

with the following software packages installed: 
- g++-5 
- make 
- libsndfile1-dev, 
- libjack-jackd2-dev
- jackd2 


4. Compilation instructions: 

After downloading and unpacking the prerelease package or cloning from github
compile the MHA with ./configure && make

4.1 Installation instructions: 

No installation routine is provided together with the source code. The binaries 
will be generated in the source tree. To collect the relevant binaries in one 
place, you may e.g. create a bin directory and copy all the binaries there with 

mkdir bin; cp $(find . -name *.so) mha/frameworks/x86_64-linux-gcc-5/mha bin 

Then, you should set the environment variable LD_LIBRARY_PATH to point to your 
bin directory, like this: 

export LD_LIBRARY_PATH=$PWD/bin 

You can also add the bin directory to the PATH environment variable: 

export PATH=$PATH:$PWD/bin 

After this, you can invoke the MHA command line. Perform a quick test with 

mha ? cmd=quit 

Which should print the default configuration of the MHA without any plugins 
loaded. 


5. Usage instructions: 

We provide with this release several examples of configuration files containing 
detailed explations. The files included are: 

a) prerelease_adm.cfg
	Adaptive Differential Microphones for signal enhancement. 
b) prerelease_coherence.cfg
	Binaural coherence filter for acoustic feedback and noise reduction. 
c) prerelease_dynamiccompression.cfg
	Multichannel dynamic range compressor for hearing loss compensation. 
d) prerelease_allplugins.cfg 
	Implementation of the full hearing aid processing chain combining all 
	plugins mentioned above. 

For example, we can start the example d) above by the following command: 

mha ?read:configurations/prerelease_allplugins.cfg cmd=start cmd=quit 

Together with these configuration files we provide also example audio data from 
a binaural four-channel hearing aid setup in an anechoic environment:

- 1speaker_diffNoise_<2ch,4ch,binaural>.wav
	A female speaker positioned 20 degree to the right of the front in a 
	spatially diffuse noise field.
- 2speaker_<2ch,4ch,binaural>.wav
	A female speaker positioned as above and a male speaker left behind the 
	head, 135 degree to the left.
- 2speaker_diffNoise_<2ch,4ch,binaural>.wav
	Both speakers in a spatially diffuse noise. 
	
The '2ch' versions are suitable input for the 'dynamiccompression' and the
'coherence' configurations, whereas the '4ch' versions can be used for the 'adm'
and the 'allplugins' configurations. (The four-channel input is reduced to two
channels by the adm processing.)

You can find the setting for the input audio files at the end in each 
configuration file - feel free to use your own audio examples. Note that for the 
adm a correct setting of the microphone distances is required for a successful
application of the algorithm.

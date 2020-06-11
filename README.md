# openMHA

HörTech Open Master Hearing Aid (openMHA)

## Content of the openMHA release 4.12.0 (2020-06-11)

The software contains the source code of the openMHA Toolbox library, of the
openMHA framework and command line application, and of a selection of algorithm
plugins forming a basic hearing aid processing chain featuring

* calibration
* bilateral adaptive differential microphones for noise suppression [1]
* binaural coherence filter for feedback reduction and dereverberation [2]
* multi-band dynamic range compressor for hearing loss compensation [3]
* spatial filtering algorithms:
	* a delay-and-sum beamformer
	 * a MVDR beamformer [4]
* single-channel noise reduction [5]
* resampling and filter plugins
* STFT cyclic aliasing prevention
* adaptive feedback cancellation [6]
* probabilistic sound source localization [7]

See below for a list of available reference implementations.

## Citation in publications

In publications using openMHA, please cite

Herzke, T., Kayser, H., Loshaj, F., Grimm, G., Hohmann, V., Open signal
processing software platform for hearing aid research (openMHA).
Proceedings of the Linux Audio Conference. Université Jean Monnet,
Saint-Étienne, pp. 35-42, 2017.

As we are working on an updated paper, please check back this section
of the README for updates.

For individual algorithms, please also refer to the list of
publications at the end of this README.

## Installation

For installation instructions for Linux, Windows and macOS, please refer
to the instructions in file INSTALLATION.md.

We also provide SD card images for Beaglebone Black with the Cape4all
sound card here: http://mahalia.openmha.org/.

## Usage instructions:

Please follow our getting-started guide:
http://www.openmha.org/docs/openMHA_starting_guide.pdf

## Known issues
### General
* analysemhaplugin does not work for io plugins.
### macOS
* There are some known issues with Octave under macOS. The openMHA gui may not work correctly with octave. As an alternative Matlab can be used.
* The jack audio plugin expects the [JackOSX distribution](http://www.jackaudio.org) to be installed. Developers wanting to use jack from other sources must compile openMHA themselves.
* The qjackctl version provided by the JackOSX distribution is rather old. The user must replace the default Server Path setting with the absolute path to jackdmp (default: /usr/local/bin/jackdmp) (May not be necessary any more, check for yourself).
* On some Apple machines Jack needs to be run with root privileges to get real-time priority.
### Windows
* On Windows 7, the openMHA Windows installer openMHA-4.12.0-installer.exe may trigger a crash report from the operating system when it exits even though openMHA was successfully installed.

## Reference algorithms

A collection of openMHA configuration files that implement signal
processing algorithms for hearing aids as they were used in the 
following publications are available in the *reference_algorithms* directory:

Baumgärtel, R. M., Krawczyk-Becker, M., Marquardt, D., Völker, C.,
Hu, H., Herzke, T., Coleman, G., Adiloğlu, K., Ernst, S. M., Gerkmann, T., 
Doclo, S., Kollmeier, B., Hohmann, V., & Dietz, M. (2015). Comparing 
Binaural Pre-processing Strategies I: Instrumental Evaluation. Trends 
in hearing, 19.
https://doi.org/10.1177/2331216515617916

and

Hendrikse, M. M. E., Grimm, G., & Hohmann, V. (2020). Evaluation of
the Influence of Head Movement on Hearing Aid Algorithm Performance
Using Acoustic Simulations. Trends in Hearing, 24, 1–20. 
https://doi.org/10.1177/2331216520916682

A database that can be utilized to reproduce the signals used in the latter
study is available under: https://doi.org/10.5281/zenodo.3621282.
 
Available methods:

* Single-channel noise reduction
* Binaural coherence filter
* Adaptive MVDR beamformer
* Binaural beamformer
* Bilateral adaptive differential microphones
* Delay-and-subtract beamformer

For references and more information see README.txt in the
 *reference_algorithms* directory.

## References for individual algorithms

[1] Elko GW, Pong ATN. A Simple Adaptive First-order Differential
Microphone. In: Proceedings of 1995 Workshop on Applications of Signal
Processing to Audio and Accoustics; 1995. p. 169–172.

[2] Grimm G, Hohmann V, Kollmeier B. Increase and Subjective
Evaluation of Feedback Stability in Hearing Aids by a Binaural
Coherence-based Noise Reduction Scheme. IEEE Transactions on Audio,
Speech, and Language Processing. 2009;17(7):1408–1419.

[3] Grimm G, Herzke T, Ewert S, Hohmann V. Implementation and
Evaluation of an Experimental Hearing Aid Dynamic Range Compressor
Gain Prescription. In: DAGA 2015; 2015. p. 996–999.

[4] Adiloğlu K, Kayser H, Baumgärtel RM, Rennebeck S, Dietz M, Hohmann
V. A Binaural Steering Beamformer System for Enhancing a Moving Speech
Source. Trends in Hearing. 2015;19:2331216515618903

[5] Gerkmann T, Hendriks RC. Unbiased MMSE-Based Noise Power
Estimation With Low Complexity and Low Tracking Delay. IEEE
Transactions on Audio, Speech, and Language
Processing. 2012;20(4):1383–1393.

[6] Schepker H, Doclo S, A semidefinite programming approach to
min-max estimation of the common part of acoustic feedback paths in
hearing aids. IEEE Transactions on Audio, Speech, and Language
Processing. 2016;24(2):366-377.

[7] Kayser H, Anemüller J, A discriminative learning approach to
probabilistic acoustic source localization. In: International Workshop
on Acoustic Echo and Noise Control (IWAENC 2014); 2014. p. 100–104.

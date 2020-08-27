## Reference algorithms

This directory contains a collection of reference configurations that
implement signal processing algorithms based on literature. These
configurations have been used in previous studies on hearing aid 
processing. For details see:

Baumgärtel, R. M., Krawczyk-Becker, M., Marquardt, D., Völker, C.,
Hu, H., Herzke, T., Coleman, G., Adiloğlu, K., Ernst, S. M., Gerkmann, T., 
Doclo, S., Kollmeier, B., Hohmann, V., & Dietz, M. (2015). Comparing 
Binaural Pre-processing Strategies I: Instrumental Evaluation. Trends 
in hearing, 19.  
https://doi.org/10.1177/2331216515617916.

and

Hendrikse, M. M. E., Grimm, G., & Hohmann, V. (2020). Evaluation of
the Influence of Head Movement on Hearing Aid Algorithm Performance
Using Acoustic Simulations. Trends in Hearing, 24, 1–20. 
https://doi.org/10.1177/2331216520916682


### Hearing aid setup

The configurations listed below assume a binaural behind-the-ear (BTE)
hearing aid microphone setup as depicted in HATS_BTE.png

Each hearing aid includes three microphones (front, middle and rear),
the distances between the microphones are given in millimeters.
The distance between the left and the corresponding right BTE-hearing
 aid microphones is 164 mm. Further details can be found in 
H. Kayser, S. D. Ewert, J. Anemüller, T. Rohdenburg, V. Hohmann, and 
B. Kollmeier (2019),“Database of Multichannel In-Ear and Behind-the-Ear 
Head-Related and Binaural Room Impulse Responses,” EURASIP Journal
on Advances in Signal Processing, Volume 2009, 10 pages, Article ID 298605
https://sirius.physik.uni-oldenburg.de/hrir/298605.pdf

### Matlab/Octave tool

A Matlab/Octave function called mha_process_ref_algo.m is available
to process an audio signal with a reference configuration. It is found 
in the mfiles directory.

## List of algorithms

The following algorithms are available:

#### delaysub
Delay-and-subtract beamformer

Input channel used:
front-left, front-right, rear-left, rear-right

Sampling rate: 48000 Hz


#### Elko1995_ADM

bilateral Adaptive differential microphone

Elko, G. W., & Pong, A. T. N. (1995, October). A simple adaptive
first-order differential microphone. In Proceedings of 1995 Workshop
on Applications of Signal Processing to Audio and Acoustics
(pp. 169-172). IEEE.

Input channel used:
front-left, front-right, rear-left, rear-right

Sampling rate: 44100 Hz


#### Rohdenburg2007_beam

Binaural beamformer

Rohdenburg, T., Hohmann, V., & Kollmeier, B. (2007,
October). Robustness analysis of binaural hearing aid beamformer
algorithms by means of objective perceptual quality measures. In
Applications of Signal Processing to Audio and Acoustics, 2007 IEEE
Workshop on (pp. 315-318). IEEE.

Input channels used:
front-left, front-right, middle-left, middle-right, rear-left, rear-right

Sampling rate: 16000 Hz


#### Baumgaertel2015_AMVDR

Adaptive MVDR beamformer

Baumgärtel, R. M., Krawczyk-Becker, M., Marquardt, D., Völker, C.,
Hu, H., Herzke, T., Coleman, G., Adiloğlu, K., Ernst, S. M., Gerkmann, T., 
Doclo, S., Kollmeier, B., Hohmann, V., & Dietz, M. (2015). Comparing 
Binaural Pre-processing Strategies I: Instrumental Evaluation. Trends 
in hearing, 19.  
https://doi.org/10.1177/2331216515617916.

including bug fix done after communication with Daniel Marquardt,
the developer of the algorithm, in the implementation in Hendrikse2020

Hendrikse, M. M. E., Grimm, G., & Hohmann, V. (2020). Evaluation of
the Influence of Head Movement on Hearing Aid Algorithm Performance
Using Acoustic Simulations. Trends in Hearing, 24, 1–20. 
https://doi.org/10.1177/2331216520916682

Input channel used:
front-left, front-right, rear-left, rear-right

Sampling rate: 16000 Hz


#### Breithaupt2008_SCNR

Single-channel noise reduction

Breithaupt, C., Gerkmann, T., & Martin, R. (2008, March). A novel a
priori SNR estimation approach based on selective cepstro-temporal
smoothing. In Acoustics, Speech and Signal Processing, 2008. ICASSP
2008. IEEE International Conference on (pp. 4897-4900). IEEE.

Input channel used:
front-left, front-right

Sampling rate: 16000 Hz


#### Grimm2009_coherence

Binaural coherence filter

Grimm, G., Hohmann, V., & Kollmeier, B. (2009). Increase and
subjective evaluation of feedback stability in hearing aids by a
binaural coherence-based noise reduction scheme. IEEE Transactions
on Audio, Speech, and Language Processing, 17(7), 1408-1419.

Input channel used:
front-left, front-right

Sampling rate: 48000 Hz

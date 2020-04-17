### delaysub
#
# Delay-and-subtract beamformer
#
# Input channel layout:
# front-left front-right rear-left rear-right
#
# Samping rate: 48000 Hz
#


### Elko1995_ADM
#
# Adaptive differntial microphone
#
# Elko, G. W., & Pong, A. T. N. (1995, October). A simple adaptive
# first-order differential microphone. In Proceedings of 1995 Workshop
# on Applications of Signal Processing to Audio and Accoustics
# (pp. 169-172). IEEE.
#
# Input channel layout:
# front-left front-right rear-left rear-right
#
# Sampling rate: 44100 Hz
#


### Rohdenburg2007_beam
#
# Binaural beamformer
#
# Rohdenburg, T., Hohmann, V., & Kollmeier, B. (2007,
# October). Robustness analysis of binaural hearing aid beamformer
# algorithms by means of objective perceptual quality measures. In
# Applications of Signal Processing to Audio and Acoustics, 2007 IEEE
# Workshop on (pp. 315-318). IEEE.
#
# Sampling rate: 16000 Hz
#
# Input channel layout:
# front-left front-right mid-left mid-right rear-left rear-right



### Baumgaertel2015_AMVDR
#
# Adaptive MVDR beamformer
#
# Baumgärtel, R. M., Krawczyk-Becker, M., Marquardt, D., Völker, C.,
# Hu, H., Herzke, T., Coleman, G., Adiloğlu, K., Ernst, S. M.,
# Gerkmann, T., Doclo, S., Kollmeier, B., Hohmann, V., & Dietz, M.
# (2015). Comparing Binaural Pre-processing Strategies I: Instrumental
# Evaluation. Trends in hearing, 19, 2331216515617916.
#
# including bug fix done after communication with Daniel Marquardt,
# the developer of the algorithm, in the implementation in Hendrikse2020
#
# Hendrikse, M. M. E., Grimm, G., & Hohmann, V. (2020). Evaluation of
# the Influence of Head Movement on Hearing Aid Algorithm Performance
# Using Acoustic Simulations. Trends in Hearing, 24,
# 1–20. https://doi.org/10.1177/2331216520916682
#
# Sampling rate: 16000 Hz
#
# Input channel layout:
# front-left front-right rear-left rear-right
#


### Breithaupt2008_SCNR
#
# Single-channel noise reduction
#
# Breithaupt, C., Gerkmann, T., & Martin, R. (2008, March). A novel a
# priori SNR estimation approach based on selective cepstro-temporal
# smoothing. In Acoustics, Speech and Signal Processing, 2008. ICASSP
# 2008. IEEE International Conference on (pp. 4897-4900). IEEE.
#
# Sampling rate: 16000 Hz
#
# Input channel layout:
# front-left front-right
#


### Grimm2009_coherence
#
# Binaural coherence filter
#
# Grimm, G., Hohmann, V., & Kollmeier, B. (2009). Increase and
# subjective evaluation of feedback stability in hearing aids by a
# binaural coherence-based noise reduction scheme. IEEE Transactions
# on Audio, Speech, and Language Processing, 17(7), 1408-1419.
#
# Sampling rate: 48000 Hz
#
# Input channel layout:
# front-left front-right
#

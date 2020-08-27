This examples demonstrates how to apply a cepstral smoothing based on noise
power spectral density.

The main plugin used is smooth_cepstrum, which does the smoothing.
The estimated psd must be provided as input via AC variable. 
The psd estimation is done by the noise_psd_estimator plugin, which does 
noise power spectral density estimation based on a cepstral-domain speech
production model using estimated speech presence probability.

References:

Colin Breithaupt, Timo Gerkmann, Rainer Martin, "A Novel A Priori SNR
Estimation Approach Based on Selective Cepstro-Temporal Smoothing", IEEE
Int. Conf. Acoustics, Speech, Signal Processing, Las Vegas, NV, USA,
Apr. 2008.

Timo Gerkmann, Rainer Martin, "On the Statistics of Spectral Amplitudes
After Variance Reduction by Temporal Cepstrum Smoothing and Cepstral
Nulling", IEEE Trans. Signal Processing, Vol. 57, No. 11, pp. 4165-4174,
Nov. 2009.

Colin Breithaupt, Timo Gerkmann, and Rainer Martin: "Spectral Smoothing
Method for Noisy Signals", European Patent EP2158588B1, granted Oct.
2010, Danish Patent DK2158588T3, granted Feb. 2011, US Patent
US8892431B2, granted Nov. 2014.

Timo Gerkmann, Richard C. Hendriks, "Unbiased MMSE-based Noise Power
Estimation with Low Complexity and Low Tracking Delay", IEEE Trans.
Audio, Speech and Language Processing, Vol. 20, No. 4, pp. 1383 - 1393,
May 2012.

Patents:

Colin Breithaupt, Timo Gerkmann, and Rainer Martin: "Spectral Smoothing
Method for Noisy Signals", European Patent EP2158588B1, granted Oct.
2010, Danish Patent DK2158588T3, granted Feb. 2011, US Patent
US8892431B2, granted Nov. 2014.

Timo Gerkmann and Rainer Martin: "Method for Determining Unbiased Signal
Amplitude Estimates After Cepstral Variance Modification", United States
Patent US8208666B2, granted Jun. 2012.

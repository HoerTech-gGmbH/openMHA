function y = fftw(x,sign)
%FFTW  Discrete Fourier transform (DFT)
%   Y = FFTW(X,SIGN) sets Y to the DFT of X, computed via a
%   fast Fourier transform algorithm. SIGN is the sign of the
%   exponent in the definition of the DFT, and should be +1 or -1.
%
%   FFTW is designed for circumstances where repeated transforms
%   of the same size are required.  The first call will incur a
%   substantial startup cost (several seconds), but subsequent
%   calls will execute very quickly.
%
%   Using SIGN = -1 corresponds to MATLAB's FFTN function.
%   Using SIGN = +1 corresponds to an unnormalized IFFTN:
%
%      FFTW(X,+1) is the same as IFFTN(X) * PROD(SIZE(X))
%
%   Thus, FFTW(FFTW(X,-1),+1) / PROD(SIZE(X)) equals X.
%
%   The input array X may have any dimensionality.  If X
%   is multi-dimensional, then a true multi-dimensional DFT
%   will be computed.
%
%   This is an interface to the FFTW C library, which is
%   described at:  http://www.fftw.org
%
%   See also: FFT, IFFT, FFT2, IFFT2, FFTN, IFFTN, FFTSHIFT

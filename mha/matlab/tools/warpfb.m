function [fy, cf, t] = warpfb( x, fftlen, rho, wtype )
% WARPFB frequency warped FFT filterbank analysis
%
% [fy, cf, t] = warp( x, fftlen, rho, wtype )
%
% Parameters:
%  x      : input signal
%  fftlen : FFT length in samples (default: 32). The number of 
%           bands is floor(fftlen/2)+1.
%  rho    : warping coefficient (default: 0.7564 = optimal coefficient
%           for bark scale at fs = 44.1 kHz)
%  wtype  : window type before fft (default: 'halfhann')
%           valid types: 'rect', 'halfhann', 'hann'
%
% Return values:
%  fy : filterbank output (FFT of y)
%  cf : center frequencies in rad
%  t  : group delay with Hanning window
%
%
% Examples:
%
%    fy = warp( chirp(0:1/44100:2,0,2,22050) );
%    plot( real(fy) );
%    plot( abs(fy) );
%    imagesc( abs(fy) );
% 
% 
% Resynthesis:
%
%  The optimal resynthesis depends on the window shape. For 'rect'
%  and 'halfhann', it can be simply the sum of real part output:
% 
%    x = sum(real(fy),2);
%
%
% Author:
%
%  Giso Grimm, May 2005

  
  if nargin < 2
    fftlen = 32;
  end
  if nargin < 3
    rho = 0.7564;
  end
  if nargin < 4
    wtype = 'halfhann';
  end
  n = floor(fftlen/2)+1;
  % selection of window type:
  if strfind(wtype,'half')==1
    wtype = wtype(5:end);
    wndlen = 2*fftlen;
    wndidx = fftlen+1:wndlen;
  else
    wndlen = fftlen;
    wndidx = 1:wndlen;
  end
  switch lower(wtype)
   case 'hann'
    wnd = hann(wndlen,'periodic')';
   case 'hamming'
    wnd = hamming(wndlen,'periodic')';
   case 'blackman'
    wnd = blackman(wndlen,'periodic')';
   case 'bartlett'
    wnd = bartlett(wndlen+1);
    wnd(end) = [];
   case 'rect'
    wnd = ones(1,wndlen);
   otherwise
    error('Invalid window type.');
  end
  wnd = wnd(wndidx);
  if size(wnd,1) > size(wnd,2)
    wnd = wnd';
  end

  % apply cascaded allpass filters B=[-rho 1] A=[1 -rho]:
  y = warped_delayline( x, fftlen, rho );
  % windowing for improved filter shapes:
  y = y .* repmat(wnd,[size(y,1) 1]);;

  % FFT and normalizing of DC band:
  fy = 2*fft(y,[],2)/fftlen;
  fy = fy(:,1:n);
  fy(:,1) = 0.5*fy(:,1);
  if 2*(n-1) == fftlen
    fy(:,n) = 0.5*fy(:,n);
  end
  
  % calculate center frequencies:
  w = 2*pi*[0:n-1]/fftlen;
  z = exp(i*w);
  cf = unwrap(angle((z+rho)./(1+z*rho)));
  
  % calculate group delay of filter bands:
  dw = pi/10000;
  z = exp(i*cf);
  zdz = exp(i*(cf+dw));
  t = -(fftlen/2)*(unwrap(angle((z-rho)./(1-z*rho))) - ...
		   unwrap(angle((zdz-rho)./(1-zdz*rho))))/dw;

  % (c) 05/2005 by Giso Grimm
  %
  % Literature:
  %
  % Smith, J.O. and Abel, J. S.: Bark and ERB Bilinear
  % Transforms. IEEE Transactions on speech and audio processing, 7
  % (6), Nov. 1999.
  %
  % A. Haermae and T. Paatero. Discrete representation of signals on a
  % logarithmic frequency scale. In Proceedings of the IEEE Workshop
  % on Applications of Signal Processing to Audio and Acoustics
  % (WASPAA'01), pages 135-138, New Paltz, NY, USA, September 2001.
  % 
  % Carlo Braccini and Alan V. Oppenheim. Unequal Bandwidth Spectral
  % Analysis using Digital Frequuency Warping. IEEE Transactions on
  % Acoustics, speech, and signal processing, Vol. ASSP-22 (4)
  % Aug. 1974.
  

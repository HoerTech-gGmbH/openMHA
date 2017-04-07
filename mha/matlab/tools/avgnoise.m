function y = avgnoise( x, fftlen, outlen )
% avgnoise - create average noise signal
%
% Usage:
%  y = avgnoise( x, fftlen, outlen );
%
% x : input vector (one column)
% y : output vector
% fftlen : FFT length used for averaging power spectrum
% outlen : length of output signal
%
% Warning: random generator not initialized!
  rmsx = sqrt(mean(x.^2));
  x = buffer( x, fftlen );
  X = sqrt(mean(abs(realfft( x )).^2,2));
  irs = realifft( X );
  irs = circshift(irs,round(fftlen/2));
  plot(irs)
  y = zeros(outlen,1);
  idx = 1:min(outlen,fftlen);
  y(idx) = irs(idx);
  Y = realfft(y);
  y = realifft(Y.*exp(i*2*pi*rand(size(Y))));
  rmsy = sqrt(mean(y.^2));
  y = y * (rmsx/rmsy);

  
  
function y = realfft( x )
% REALFFT - FFT transform of pure real data
%
% Usage: y = realfft( x )
%
% Returns positive frequencies of fft(x), assuming that x is pure
% real data. Each column of x is transformed separately.
  ;
  fftlen = size(x,1);
  
  y = fft(x);
  y = y([1:floor(fftlen/2)+1],:);

  
function x = realifft( y )
% REALIFFT - inverse FFT of positive frequencies in y
%
% Usage: x = realifft( y )
%
% Returns inverse FFT or half-complex spectrum. Each column of y is
% taken as a spectrum.
  ;
  channels = size(y,2);
  nbins = size(y,1);
  x = zeros(2*(nbins-1),channels);
  for ch=1:channels
    ytmp = y(:,ch);
    ytmp(1) = real(ytmp(1));
    ytmp(nbins) = real(ytmp(nbins));
    ytmp2 = [ytmp; conj(ytmp(nbins-1:-1:2))];
    x(:,ch) = real(ifft(ytmp2));
  end

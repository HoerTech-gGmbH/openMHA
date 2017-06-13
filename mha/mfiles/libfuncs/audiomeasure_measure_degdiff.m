function dd = audiomeasure_measure_degdiff( x, fs, fftlen, wndlen )
% degree of diffusiveness after Wittkop (2001)
%
% Usage:
% dd = measure_degdiff( x, fs, fftlen, wndlen )
%
% x : input signal (Nx2)
% fs : sampling rate / Hz (orig: 25000)
% fftlen : fft length (orig: 512)
% wndlen : window length (orig: 400)
% dd : degree of diffusiveness
%
% time constants:
% t_MSC = 0.04
% t_dd = signal length (orig: 5s)
  vF = ([1:floor(fftlen/2-1)]'-1)/fftlen*fs;
  Xl = stft(x(:,1),fftlen,wndlen);
  Xr = stft(x(:,2),fftlen,wndlen);
  frate = fs/(0.5*wndlen);
  [B,A] = butter(1,0.04/(0.5*frate));
  MSC = abs(filter(B,A,Xl.*conj(Xr))).^2./max(1e-10,(filter(B,A,abs(Xl).^2) .* filter(B,A,abs(Xr).^2)));
  w_f = ones(size(MSC));
  w_f(vF<1000,:) = 0;
  dd = mean(mean(w_f .* (1-sqrt(MSC)),1));
  
  
function X = stft( x, fftlen, wndlen )
  zpadlen = fftlen-wndlen;
  zpad1 = floor(zpadlen/2);
  zpad2 = zpadlen - zpad1;
  xb = buffer(x, wndlen, wndlen/2 );
  xb = [zeros(zpad1,size(xb,2));...
	xb .* repmat(hann(wndlen),[1,size(xb,2)]);...
	zeros(zpad2,size(xb,2))];
  X = realfft(xb);

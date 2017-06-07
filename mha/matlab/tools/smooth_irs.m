function [smooth_irs,Hcomp] = smooth_irs(irs,fs,wndwlength,a,f)
% [smooth_irs,Hcomp] = smooth_irs(irs,fs,wndwlength,a,f)
%
% wndlength   length of a hanning window that is applied to cut off non-linear resonances in the measured impulse response.
% a           strength of the limiting of the compensation spectrum for high frequencies.
% f           start frequency of the limiting of the compensation spectrum for high frequencies.

% initialise with defaults
if nargin < 5
  f = 7000;
  if nargin < 4
    a = 1/6000;
    if nargin < 3
      wndwlength = 1000;
    end
  end
end

irs(end+1:fs,:) = 0; % Sets x-axis units to Hz
wndw = [hanning(2*wndwlength)(wndwlength+1:end); zeros(length(irs)-wndwlength,1)];
irs .*= wndw; % cut off non-linearities
Hcomp = abs(realfft(irs)) .^ (-1);

Hcomp_classic = Hcomp; %%%%%%%%%%%%%% testing only

[A_min,f_min] = min(Hcomp(1:500));   % find resonance frequency of the speaker. ('findpeaks' didn't work.)
Hcomp(1:f_min)=A_min;                % cut off bass droop

m = median(Hcomp(floor(f/2):end));

Hcomp(f:end) -= max( (Hcomp(f:end)-m) .* (1-exp( a*[0:-1:-(size(Hcomp)(1)-f)]' )) , 0);

plot(20*log10([Hcomp_classic, Hcomp])) %%%%%%%%%%%%%% testing only

smooth_irs = abs(realfft(Hcomp));

end
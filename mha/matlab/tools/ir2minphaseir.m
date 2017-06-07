function ir = ir2minphaseir( ir, nmax )
% ir2minphaseir - generate minimum phase impulse response
%
% Usage:
% ir = ir2minphaseir( ir [, nmax ] );
  
  N = size(ir,1);
  if nargin < 2
    nmax = N;
  end
  H = abs(realfft(ir));
  Nbins = size(H,1);
  P = log(max(1e-10,H));
  P(Nbins+1:N,:) = P(Nbins-1:-1:2,:);
  P = imag(hilbert(P));
  ir = realifft(H.*exp(-i*P(1:Nbins,:)));
  ir(nmax+1:end,:) = [];
end
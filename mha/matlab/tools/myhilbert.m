function x = myhilbert( xr )
  if prod(size(xr)) == max(size(xr))
    xr = xr(:);
  end
  X = fft(xr);
  fftlen = size(X,1);
  nyquist_bin = floor(fftlen/2)+1;
  X(nyquist_bin+1:end,:) = 0;
  X(1,:) = 0.5*X(1,:);
  if ~mod(fftlen,2)
    X(nyquist_bin,:) = 0.5*X(nyquist_bin,:);
  end
  x = ifft(2*X);
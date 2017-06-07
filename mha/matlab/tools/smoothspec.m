function irs = smoothspec( spec, n )
  numbins = size(spec,1);
  if ~bitand( numbins, 1 )
    error(['This function works only for even FFT lengths\n',...
	   '(odd number of bins in spectrum)']);
  end
  fftlen = 2*(numbins-1);
  spec = abs(spec);
  phase = -imag(myhilbert(log(max(1e-10,spec))));
  irs = realifft(spec .* exp(i*phase));
  wnd = cos(0.5*[0:(n-1)]'*pi/n);
  irs = irs(1:n,:) .* repmat(wnd,[1,size(spec,2)]);

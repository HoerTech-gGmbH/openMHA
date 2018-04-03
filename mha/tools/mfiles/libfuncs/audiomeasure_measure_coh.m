function c = audiomeasure_measure_coh( s1, s2, N )
  s1 = buffer(s1,N);
  s2 = buffer(s2,N);
  s1 = fft(s1,[],2);
  s2 = fft(s2,[],2);
  Z = s1.*conj(s2);
  Z = Z ./ abs(Z);
  c = mean(abs(mean(Z,2)));
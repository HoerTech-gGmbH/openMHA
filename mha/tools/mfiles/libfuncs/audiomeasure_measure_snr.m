function snr = audiomeasure_measure_snr( s, n )
  snr = 10*log10(mean(s.^2)./mean(n.^2));
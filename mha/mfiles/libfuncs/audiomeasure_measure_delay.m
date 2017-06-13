function d = audiomeasure_measure_delay( x, y )
  [c,l] = xcorr(x,y,2048);
  [tmp,idx] = max(c);
  d = l(idx);
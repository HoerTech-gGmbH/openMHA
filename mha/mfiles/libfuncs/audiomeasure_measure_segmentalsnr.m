function snr = audiomeasure_measure_segmentalsnr( s, n, fs )
  ;
  
  % 125 ms blocks:
  Nchunk = round(fs*0.125);
  
  snr = [];
  for k=1:size(s,2)
    Sbuf = buffer(s(:,k),Nchunk);
    Nbuf = buffer(n(:,k),Nchunk);
    % short time SNR:
    st_SNR = 10*log10(mean(Sbuf.^2)./max(eps,mean(Nbuf.^2)));
    st_SNR = min(35,max(-20,st_SNR));
    snr(k) = mean(st_SNR);
  end

function q = audiomeasure_measure_dau2010( SNproc, Sref )
  % measure after Christiansen et al. (2010)
  % requires medi-modules 'gammatone_filterbank' and 'adapt_loop'
  Pref = pemo( Sref, 20480 );
  Pproc = pemo( SNproc, 20480 );
  RMS_tot = 10*log10(mean(Sref(:).^2));
  Nshift = 205;
  Nblocks = floor(size(Sref,1)/Nshift)-1;
  vRMS = zeros(Nblocks,1);
  vCorr = zeros(Nblocks,1);
  for k=1:Nblocks
    idx = (k-1)*Nshift+[1:2*Nshift];
    vRMS(k) = 10*log10(mean(Sref(idx).^2))-RMS_tot;
    vCorr(k) = corr(mean(Pref(idx,:))',mean(Pproc(idx,:))');
  end
  idx_low = find((vRMS<=-5) & (vRMS>=-15));
  idx_mid = find((vRMS<=0) & (vRMS>-5));
  idx_high = find((vRMS>0));
  r_low = mean(vCorr(idx_low));
  r_mid = mean(vCorr(idx_mid));
  r_high = mean(vCorr(idx_high));
  % model parameters:
  w_low = 0;
  w_mid = 0;
  w_high = 1;
  S = 0.056;
  O = 0.39;
  c = w_low*r_low + w_mid*r_mid + w_high*r_high;
  q = 1./(1+exp((O-c)/S));
  
function Y = pemo( x, fs )
  analyzer = Gfb_Analyzer_new(fs,100,1000,8000,1);
  Y = real(Gfb_Analyzer_fprocess(analyzer, x')');
  for k=1:size(Y,2)
    Y(:,k) = haircell(Y(:,k), fs );
    Y(:,k) = adapt_m( Y(:,k), fs );
  end

  
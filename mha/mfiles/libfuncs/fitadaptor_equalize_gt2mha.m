function sCfg = fitadaptor_equalize_gt2mha( sGt, sMHACfg )
  sCfg = struct;
  fftlen = sMHACfg.mhaconfig_in.fftlen;
  fftfreqs = [0:fftlen] / fftlen * sMHACfg.mhaconfig_in.srate;
  bins = floor(fftlen/2)+1;
  % one row for each level, one column for each frequency band:
  sCfg.gains = [];
  for ch=sGt.side
    sCfg.gains = [sCfg.gains;10.^(sGt.(ch)(2,:)/20)];
  end

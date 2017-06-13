function sCfg = fitadaptor_spsmft_gt2mha( sGt, sMHACfg )
  sCfg = struct;
  % one row for each level, one column for each frequency band:
  switch sGt.side
    case 'l'
     sCfg.GT.gain_L = sGt.l';
     sCfg.GT.gain_R = sGt.l';
   case 'r'
    sCfg.GT.gain_L = sGt.r';
    sCfg.GT.gain_R = sGt.r';
   case 'lr'
    sCfg.GT.gain_L = sGt.l';
    sCfg.GT.gain_R = sGt.r';
   case 'rl'
    sCfg.GT.gain_L = sGt.r';
    sCfg.GT.gain_R = sGt.l';
  end
  sCfg.GT.gainrule = sGt.fit.gainrule;
  sCfg.GT.clientid = sGt.fit.audprof.client_id;

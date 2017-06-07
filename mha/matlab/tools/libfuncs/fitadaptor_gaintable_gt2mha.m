function sCfg = fitadaptor_gaintable_gt2mha( sGt, sMHACfg )
  sCfg = struct;
  % one row for each level, one column for each frequency band:
  switch sGt.side
    case 'l'
     sCfg.gain_L = sGt.l';
     sCfg.gain_R = sGt.l';
   case 'r'
    sCfg.gain_L = sGt.r';
    sCfg.gain_R = sGt.r';
   case 'lr'
    sCfg.gain_L = sGt.l';
    sCfg.gain_R = sGt.r';
   case 'rl'
    sCfg.gain_L = sGt.r';
    sCfg.gain_R = sGt.l';
  end
  sCfg.gainrule = sGt.fit.gainrule;
  sCfg.clientid = sGt.fit.audprof.client_id;

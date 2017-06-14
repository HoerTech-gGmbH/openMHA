function sCfg = fitadaptor_dc_simple_gt2mha( sGt, sMHACfg )
  sCfg = struct;
  % one row for each level, one column for each frequency band:
  sCfg.g50 = [];
  sCfg.g80 = [];
  sCfg.expansion_threshold = [];
  sCfg.expansion_slope = [];
  for ch=sGt.side
    sCfg.g50 = [sCfg.g50,sGt.(ch)(1,:)];
    sCfg.g80 = [sCfg.g80,sGt.(ch)(2,:)];
    sCfg.expansion_threshold = [sCfg.expansion_threshold,sGt.noisegate.(ch).level(:)'];
    sCfg.expansion_slope = [sCfg.expansion_slope,sGt.noisegate.(ch).slope(:)'];
  end
  sCfg.gainrule = sGt.fit.gainrule;
  sCfg.clientid = sGt.fit.audprof.client_id;

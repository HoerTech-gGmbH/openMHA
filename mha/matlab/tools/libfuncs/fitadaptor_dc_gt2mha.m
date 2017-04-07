function sCfg = fitadaptor_dc_gt2mha( sGt, sMHACfg )
  sCfg = struct;
  sCfg.gtdata = [];
  sCfg.gtmin = [];
  sCfg.gtstep = [];
  for ch=sGt.side
    sCfg.gtdata = [sCfg.gtdata;sGt.(ch)'];
    sCfg.gtmin = [sCfg.gtmin,repmat(min(sGt.levels),[1,length(sGt.frequencies)])];
    sCfg.gtstep = [sCfg.gtstep,repmat(1,[1,length(sGt.frequencies)])];
  end
  sCfg.gainrule = sGt.fit.gainrule;
  sCfg.clientid = sGt.fit.audprof.client_id;

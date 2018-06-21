function sCfg = fitadaptor_ohcsimple_gt2mha( sGt, sMHACfg )
  if ~isfield(sGt,'compression')
    error('The selected gainrule does not provide parametrized IO function');
  end
  sCfg = struct;
  % one row for each level, one column for each frequency band:
  vsChPlug='LR';
  csPar={'gain','l_kneepoint','c_slope'};
  for k=1:2
    sChGt=sGt.side(min(k,numel(sGt.side)));
    sChPlug = vsChPlug(k);
    for sPar=csPar
      sCfg.([sChPlug,'_compression']).(sPar{:}) = sGt.compression.(sChGt).(sPar{:});
    end
  end
  sCfg.gainrule = sGt.fit.gainrule;
  sCfg.clientid = sGt.fit.audprof.client_id;

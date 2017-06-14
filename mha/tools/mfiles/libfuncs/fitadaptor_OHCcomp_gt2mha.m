function sCfg = fitadaptor_OHCcomp_gt2mha( sGt, sMHACfg )
  if ~isfield(sGt,'basilar_io')
    error('The selected gainrule does not provide basilar membrane IO parameters');
  end
  sCfg = struct;
  % one row for each level, one column for each frequency band:
  vsChPlug='LR';
  csPar={'gmax_n','gmax_i','l_passive','c_slope'};
  for k=1:2
    sChGt=sGt.side(min(k,numel(sGt.side)));
    sChPlug = vsChPlug(k);
    for sPar=csPar
      sCfg.([sChPlug,'_',sPar{:}]) = sGt.basilar_io.(sChGt).(sPar{:});
    end
  end
  sCfg.gainrule = sGt.fit.gainrule;
  sCfg.clientid = sGt.fit.audprof.client_id;

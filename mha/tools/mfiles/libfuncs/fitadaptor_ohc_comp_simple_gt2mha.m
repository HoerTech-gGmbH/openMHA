function sCfg = fitadaptor_ohc_comp_simple_gt2mha( sGt, sMHACfg )
  if ~isfield(sGt,'compression')
    error('The selected gainrule does not provide basilar membrane IO parameters');
  end
  sCfg = struct;
  % one row for each level, one column for each frequency band:
  vsChPlug='LR';
  csPar={'gain','l_kneepoint','c_slope'};
  warning('works for 1 channel only! Do not use.')
  for k=1
    sChGt=sGt.side(min(k,numel(sGt.side)));
    sChPlug = vsChPlug(k);
    for sPar=csPar
      sCfg.compression.(sPar{:}) = sGt.compression.(sChGt).(sPar{:});
    end
  end
  
  
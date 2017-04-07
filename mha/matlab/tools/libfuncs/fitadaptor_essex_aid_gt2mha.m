function sCfg = fitadaptor_essex_aid_gt2mha( sGt, sMHACfg )
  if ~isfield(sGt,'essex_io')
    error('The selected gainrule does not provide Essex IO parameters');
  end
  sCfg = struct;

  sCfg.inst_compr_threshold = sGt.essex_io.tc;
  sCfg.gain = sGt.essex_io.gain;
  sCfg.moc_tc = sGt.essex_io.tm;
  sCfg.moc_factor = sGt.essex_io.moc_factor;
  sCfg.moc_tc = sGt.essex_io.moc_tau;


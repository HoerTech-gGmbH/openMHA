function sFitModel = fitadaptor_spsmft_mha2fitmodel( sMHACfg )
  sFitModel = struct;
  sFitModel.frequencies = sMHACfg.IFest.f_hz;
  cf = sFitModel.frequencies;
  if length(cf) == 1
    ef = [125,8000];
  else
    ef = sqrt(cf(2:end).*cf(1:end-1));
    ef = [cf(1)*cf(1)/ef(1),ef,cf(end)*cf(end)/ef(end)];
  end
  sFitModel.edge_frequencies = ef;
  sFitModel.levels = sMHACfg.GT.Linput;
  sFitModel.channels = sMHACfg.mhaconfig_in.channels/length(sMHACfg.IFest.f_hz);

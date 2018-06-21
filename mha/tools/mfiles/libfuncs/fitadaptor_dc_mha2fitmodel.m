function sFitModel = fitadaptor_dc_mha2fitmodel( sMHACfg )
  sFitModel = struct;
  sFitModel.frequencies = sMHACfg.cf;
  sFitModel.edge_frequencies = sMHACfg.ef;
  sFitModel.levels = -10:1:110;
  sFitModel.channels = sMHACfg.mhaconfig_in.channels/length(sMHACfg.cf);

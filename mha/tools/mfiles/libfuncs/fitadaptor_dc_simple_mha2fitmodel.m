function sFitModel = fitadaptor_dc_simple_mha2fitmodel( sMHACfg )
  sFitModel = struct;
  sFitModel.frequencies = sMHACfg.cf;
  sFitModel.edge_frequencies = sMHACfg.ef;
  sFitModel.levels = [50 80];
  sFitModel.channels = sMHACfg.mhaconfig_in.channels/length(sMHACfg.cf);

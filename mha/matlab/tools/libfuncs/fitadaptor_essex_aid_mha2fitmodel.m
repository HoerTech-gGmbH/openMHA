function sFitModel = fitadaptor_essex_aid_mha2fitmodel( sMHACfg )

  sFitModel = struct;
  sFitModel.edge_frequencies = sMHACfg.edge_frequencies;
  sFitModel.frequencies = sqrt(sFitModel.edge_frequencies(2:end) .* ...
			       sFitModel.edge_frequencies(1:(end-1)));
  sFitModel.levels = [0:1:100];
  sFitModel.channels = sMHACfg.mhaconfig_in.channels;

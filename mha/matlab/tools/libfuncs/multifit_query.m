function csPlugs = multifit_query( mha )
  global mha_basic_cfg;
  mha_get_basic_cfg_network( mha );
  csPlugs = {};
  lfa = libfitadaptor();
  for kPlug=1:size(mha_basic_cfg.all_id_plugs,1)
    sAddr = mha_basic_cfg.all_id_plugs{kPlug,1};
    sPlug = mha_basic_cfg.all_id_plugs{kPlug,2};
    sMultifitRule1 = [sPlug,'_mha2fitmodel'];
    sMultifitRule2 = [sPlug,'_gt2mha'];
    if isfield(lfa,sMultifitRule1) && isfield(lfa,sMultifitRule2)
      sCfg = struct;
      sCfg.addr = sAddr;
      sCfg.plugin = sPlug;
      sCfg.plugincfg = mha_get(mha,sAddr);
      sCfg.mha2fitmodel = lfa.(sMultifitRule1);
      sCfg.gaintable2mha = lfa.(sMultifitRule2);
      sCfg.fitmodel = sCfg.mha2fitmodel(sCfg.plugincfg);
      csPlugs{end+1} = sCfg;
    end
  end

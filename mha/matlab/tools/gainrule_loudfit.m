function sGt = gainrule_loudfit( sAud, sCfg )
  sAud = audprof2aud( sAud );
  vNH_UCL = [106.9 106.9 108.5 103.4  99.4  99.5  96.6  94.5  97.9 ...
	     102.4  94.9];
  vUCLFreq =  [125,250,500,750,1000,1500,2000,3000,4000,6000,8000];
  UCL = interp1(log(vUCLFreq),vNH_UCL,log(sAud.frequencies),'linear','extrap');

  sAud.l.ucl(:) = UCL;
  sAud.r.ucl(:) = UCL;
  sAud.name = 'loudfit';
  sCfg.freq = sCfg.frequencies;
  sCfg.hl_correction = 'F';
  sCfg.skip_figures = 1;
  sCfg.audiogram = sAud;
  sCfg.expansion_slope = 1;
  sCfg.loudness_src = 'aud';
  sGtBasic = gt_basic( sCfg );
  sGt = struct;
  sGt.l = sGtBasic.l';
  sGt.r = sGtBasic.r';

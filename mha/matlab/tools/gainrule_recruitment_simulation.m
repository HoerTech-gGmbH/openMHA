function sGt = gainrule_recruitment_simulation( sAud, sCfg )
  sAud = audprof2aud( sAud );
  vNH_UCL = [106.9 106.9 108.5 103.4  99.4  99.5  96.6  94.5  97.9 ...
	     102.4  94.9];
  sAud.l.ucl(:) = vNH_UCL;
  sAud.r.ucl(:) = vNH_UCL;
  %sAud = limit_to_50_plus_30( sAud );
  sAud.name = 'loudfit';
  sCfg.freq = sCfg.frequencies;
  sCfg.hl_correction = 'F';
  sCfg.skip_figures = 1;
  sCfg.audiogram = sAud;
  sCfg.expansion_slope = 1;
  sCfg.loudness_src = 'aud';
  sCfg.recruitment = 1;
  sGtBasic = gt_basic( sCfg );
  sGt = struct;
  sGt.l = sGtBasic.l';
  sGt.r = sGtBasic.r';
  %sGt.expansion_slope = 2*ones(1,length(sCfg.frequencies));
  
function sAud = limit_to_50_plus_30( sAud )
  for csSide={'l','r'}
    sSide = csSide{:};
    htl = sAud.(sSide).htl;
    idx = find(htl>50);
    htl(idx) = (htl(idx)-50.0)*0.3+50.0;
    sAud.(sSide).htl = htl;
  end


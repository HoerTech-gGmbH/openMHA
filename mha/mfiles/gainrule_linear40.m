function sGt = gainrule_linear40( sAud, sCfg )
% GAINRULE_LINEAR40 - Prescribe 40% of hearing loss as insertion
% gain (no compression)
  
  nLev = length(sCfg.levels);
  sGt = struct;
  libaudprof();
  for side=sCfg.side
    sT = audprof.threshold_get( sAud, side, 'htl_ac' );
    htl = freq_interp_sh([sT.data.f],[sT.data.hl],...
			 sCfg.frequencies);
    sGt.(side) = repmat(0.4*htl,[nLev 1]);
    sGt.noisegate.(side).level = 35*ones(size(sCfg.frequencies));
    sGt.noisegate.(side).slope = ones(size(sCfg.frequencies));
  end

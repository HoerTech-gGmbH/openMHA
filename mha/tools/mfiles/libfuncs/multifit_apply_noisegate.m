function sGt = multifit_apply_noisegate( sGt )
  if isfield( sGt, 'noisegate' )
    for ch='lr'
      for kf=1:length(sGt.frequencies)
	Gain_Noisegate = interp1(sGt.levels,sGt.(ch)(:,kf), ...
				 sGt.noisegate.(ch).level(kf),'linear','extrap');
	idx = find(sGt.levels<sGt.noisegate.(ch).level(kf));
	sGt.(ch)(idx,kf) = (sGt.levels(idx)-sGt.noisegate.(ch).level(kf))* ...
	    sGt.noisegate.(ch).slope(kf) + Gain_Noisegate;
      end
    end
  end

function sAud = audprof_audprof_average( csAud, avgfun )
  sAud = audprof_audprof_new();
  if isempty(csAud)
    return;
  end
  if nargin < 2
    avgfun = @(x) mean(x,1);
  end
  for side='lr'
    for cType={'htl_ac','htl_bc','ucl'}
      stype = cType{:};
      sTh = audprof_threshold_get( csAud{1}, side, stype );
      vf = [sTh.data.f];
      for k=2:numel(csAud)
	sTh = audprof_threshold_get( csAud{k}, side, stype );
	vf = intersect(vf,[sTh.data.f]);
      end
      if ~isempty(vf)
	mH = zeros([numel(csAud),numel(vf)]);
	for k=1:numel(csAud)
	  sTh = audprof_threshold_get( csAud{k}, side, stype );
	  sTh = audprof_threshold_fill_intersect( sTh, vf, zeros(size(vf)));
	  mH(k,:) = [sTh.data.hl];
	end
	sAud.(side).(stype) = ...
	    audprof_threshold_entry_add([],vf, avgfun(mH));
      end
    end
  end
